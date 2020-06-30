/*
 * Copyright 2012, 2017 INFN
 *
 * Licensed under the EUPL, Version 1.2 or – as soon they
 * will be approved by the European Commission - subsequent
 * versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 * Licence.
 * You may obtain a copy of the Licence at:
 *
 * https://joinup.ec.europa.eu/software/page/eupl
 *
 * Unless required by applicable law or agreed to in
 * writing, software distributed under the Licence is
 * distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied.
 * See the Licence for the specific language governing
 * permissions and limitations under the Licence.
 */
#include <chaos/common/configuration/GlobalConfiguration.h>
#include <chaos/common/async_central/AsyncCentralManager.h>
#include <chaos/common/utility/TimingUtil.h>



#include <boost/array.hpp>
#include <boost/asio.hpp>

#include <stdlib.h>

using namespace chaos::common::utility;
using namespace chaos::common::async_central;

int64_t TimingUtil::timestamp_calibration_offset = 0;
int64_t TimingUtil::mds_calibration_offset = 0;

int32_t TimingUtil::timestamp_uncertenty_mask=0;
const char* TimingUtil::formats[]={
    "%Y-%m-%dT%H:%M:%S.%fZ",
    "%Y-%m-%d %H:%M:%S.%fZ",
    "%Y-%m-%dT%H:%M:%S.%f",
    "%Y-%m-%d %H:%M:%S.%f",
    "%Y-%m-%d %H:%M:%SZ",
    "%Y-%m-%dT%H:%M:%SZ",
    "%Y-%m-%d %H:%M:%S",
    "%Y-%m-%dT%H:%M:%S",
    "%Y-%m-%d %H:%MZ",
    "%Y-%m-%dT%H:%MZ",
    "%Y-%m-%d %H:%M",
    "%Y-%m-%dT%H:%M",
    "%Y-%m-%d %HZ",
    "%Y-%m-%dT%HZ",
    "%Y-%m-%d %H",
    "%Y-%m-%dT%H",
    "%Y-%m-%dZ",
    "%Y-%m-%d",
    "%Y-%mZ",
    "%Y-%m",
    "%YZ",
    "%Y"
};

const size_t TimingUtil::formats_n = 11;

TimingUtil::TimingUtil():
calibration_enable_status(GlobalConfiguration::getInstance()->getOption<bool>(InitOption::OPT_TIME_CALIBRATION)),
calibration_offset_bound(GlobalConfiguration::getInstance()->getOption<uint32_t>(InitOption::OPT_TIME_CALIBRATION_OFFSET_BOUND)),
remote_ntp_server(GlobalConfiguration::getInstance()->getOption<std::string>(InitOption::OPT_TIME_CALIBRATION_NTP_SERVER)){}


void TimingUtil::enableTimestampCalibration() {
    const bool has_ntp_server = GlobalConfiguration::getInstance()->hasOption(InitOption::OPT_TIME_CALIBRATION_NTP_SERVER);
    if(calibration_enable_status == false) throw CException(-1, "Calibration is not enable for this process", __PRETTY_FUNCTION__);
    if(has_ntp_server == false) throw CException(-2, "Calibration need to have an ntp server host name", __PRETTY_FUNCTION__);
    if(remote_ntp_server.size() == 0) throw CException(-3, "NTP server need to be valid", __PRETTY_FUNCTION__);
    
    AsyncCentralManager::getInstance()->addTimer(this, 0, 60000);
}

void TimingUtil::disableTimestampCalibration() {
    AsyncCentralManager::getInstance()->removeTimer(this);
}
 
 uint64_t TimingUtil::getTimestampFromString(const std::string& timestamp,const std::string& format,bool isutc) {
                    boost::posix_time::ptime timep;
                    size_t i=0;
                    std::istringstream is(timestamp);
                    boost::posix_time::time_input_facet *f=new boost::posix_time::time_input_facet(format);
                    std::locale loc=std::locale(std::locale::classic(), f);
                    is.imbue(loc);
                    is >> timep;

                    if(isutc){
                        return (timep-EPOCH).total_milliseconds();
                    }
                    return ((timep-getUTCOffset())-EPOCH).total_milliseconds();
                    
}

 std::string TimingUtil::toString(uint64_t since_epoc_ms,
                                                   const std::string& format,bool isUtc) {
                    boost::posix_time::time_facet* facet=new boost::posix_time::time_facet(format.c_str());
                    std::ostringstream stream;
                    stream.imbue(std::locale(stream.getloc(), facet));
                    boost::posix_time::ptime p(EPOCH + boost::posix_time::milliseconds(since_epoc_ms));
                    stream << ((isUtc)?local_adj::utc_to_local(p):p);
                    return stream.str();
}

  std::string TimingUtil::toStringFromMicroseconds(uint64_t since_epoc_us,
                                                   const std::string& format,bool isUtc) {
                    boost::posix_time::time_facet* f=new boost::posix_time::time_facet (format.c_str());
                    std::ostringstream stream;

                    stream.imbue(std::locale(stream.getloc(), f));
                    boost::posix_time::ptime p(EPOCH + boost::posix_time::microseconds(since_epoc_us));

                    stream <<  ((isUtc)?local_adj::utc_to_local(p):p);
                    return stream.str();
}
void TimingUtil::timeout() {
    uint64_t ntp_rx_ts = 0;
    uint64_t ntp_tx_ts = 0;
    try{
        uint64_t pre_ts = getTimeStamp();
        getNTPTS(ntp_rx_ts, ntp_tx_ts);
        uint64_t post_ts = getTimeStamp();
        //uint64_t post_ts = getTimeStamp();
        int64_t diff_rx = ntp_rx_ts - pre_ts; //how much ntp time diff from us
        int64_t diff_tx = ntp_tx_ts - post_ts;
        int64_t offset = (int64_t)((double)(diff_rx + diff_tx) / 2.0);
        //double delay = (post_ts-pre_ts) - (ntp_tx_ts-ntp_rx_ts);
        if(((int64_t)std::fabs(offset)) > calibration_offset_bound) {
            timestamp_calibration_offset += (uint64_t)offset;
            TU_LAPP << CHAOS_FORMAT("Timestamp calibration has been applyed for %1% milliseconds with pre_ts:%2% ntptx:%3%",%offset%pre_ts%ntp_tx_ts);
        } else {
            TU_LDBG << CHAOS_FORMAT("Timestamp calibration acquired data pre_ts:%1% ntptx:%2% diff:%3% ", %pre_ts%ntp_tx_ts%offset);
        }
    } catch(CException& ex) {
        DECODE_CHAOS_EXCEPTION(ex);
    }
    
}

#define COMPUTE_NTP_TS(s,f)\
((((uint64_t)s - 2208988800ULL)  * 1000) + (uint64_t)(((double)f / 4294967296.0) * 1000))

void TimingUtil::getNTPTS(uint64_t& ntp_received_ts,
                          uint64_t& ntp_reansmitted_ts) {
    //time_t result_time = 0;
    boost::array<char, 48> asio_buffer;
    ntp_packet *packet = reinterpret_cast<ntp_packet*>(asio_buffer.data());
    //reset ntp packet
    memset( asio_buffer.data(), 0, 48 );
    *( ( char * ) asio_buffer.data() + 0 ) = 0x1b; //00011011
    
    boost::asio::io_service io_service;
    boost::asio::ip::udp::resolver resolver(io_service);
    boost::asio::ip::udp::resolver::query query(boost::asio::ip::udp::v4(),
                                                remote_ntp_server,
                                                "ntp");
    
    boost::asio::ip::udp::endpoint receiver_endpoint = *resolver.resolve(query);
    
    boost::asio::ip::udp::socket socket(io_service);
    socket.open(boost::asio::ip::udp::v4());
    
    
    
    socket.send_to(boost::asio::buffer(asio_buffer), receiver_endpoint);
    boost::asio::ip::udp::endpoint sender_endpoint;
    
    try{
        size_t len = socket.receive_from(boost::asio::buffer(asio_buffer),
                                         sender_endpoint);
        if(len != sizeof(ntp_packet)) {throw chaos::CException(-1, "Error receiving data from ntp", __PRETTY_FUNCTION__);}
        packet->rxTm_s = ntohl(packet->rxTm_s);
        packet->rxTm_f = ntohl(packet->rxTm_f);
        packet->txTm_s = ntohl(packet->txTm_s);
        packet->txTm_f = ntohl(packet->txTm_f);
        //const uint64_t sec_part = ((uint64_t)packet->txTm_s - 2208988800ULL)  * 1000; //seconds since 1970
        //const double msec_part = ((double)packet->txTm_f / /*(double)std::numeric_limits<int32_t>::max()*/4294967296.0) * 1000;
        //timeRecv = recvBuf[43] << 24 | recvBuf[42] << 16 | recvBuf[41] << 8 | recvBuf[40];
        //timeRecv = htonll((time_t)timeRecv);
        ntp_received_ts = COMPUTE_NTP_TS(packet->rxTm_s, packet->rxTm_f);
        ntp_reansmitted_ts = COMPUTE_NTP_TS(packet->txTm_s, packet->txTm_f);//(((uint64_t)packet->txTm_s - 2208988800ULL)  * 1000) + (uint64_t)(((double)packet->txTm_f / 4294967296.0) * 1000);
        
    } catch (std::exception& e){
        throw chaos::CException(-2, e.what(), __PRETTY_FUNCTION__);
        
    }
}
