/*
 * Copyright 2012, 2019 INFN
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
#include <chaos/common/metric/MetricManager.h>

using namespace chaos::common::metric;

using namespace prometheus;
#pragma mark CCounter
chaos::common::metric::CCounter::CCounter(prometheus::Counter& _impl):
impl(_impl){}

double chaos::common::metric::CCounter::operator()() {
    return impl.Value();
}
prometheus::Counter& chaos::common::metric::CCounter::operator++() {
    impl.Increment();
    return impl;
}
prometheus::Counter& chaos::common::metric::CCounter::operator++(int) {
    impl.Increment();
    return impl;
}
double chaos::common::metric::CCounter::operator+(const double d) const {
    impl.Increment(d);
    return impl.Value();
}
chaos::common::metric::CCounter& chaos::common::metric::CCounter::operator+=(const double d) {
    impl.Increment(d);
    return *this;
}
#pragma mark CGauge
chaos::common::metric::CGauge::CGauge(prometheus::Gauge& _impl):
impl(_impl){}

double chaos::common::metric::CGauge::operator()() {
    return impl.Value();
}

prometheus::Gauge& chaos::common::metric::CGauge::operator--() {
    impl.Decrement();
    return impl;
}

prometheus::Gauge& chaos::common::metric::CGauge::operator++() {
    impl.Increment();
    return impl;
}

prometheus::Gauge& chaos::common::metric::CGauge::operator--(int) {
    impl.Decrement();
    return impl;
}

prometheus::Gauge& chaos::common::metric::CGauge::operator++(int) {
    impl.Increment();
    return impl;
}
double chaos::common::metric::CGauge::operator+(const double d) const {
    impl.Increment(d);
    return impl.Value();
}
double chaos::common::metric::CGauge::operator-(const double d) const {
    impl.Decrement(d);
    return impl.Value();
}
double chaos::common::metric::CGauge::operator=(const double d) const {
    impl.Set(d);
    return impl.Value();
}
chaos::common::metric::CGauge& chaos::common::metric::CGauge::operator+=(const double d) {
    impl.Increment(d);
    return *this;
}

chaos::common::metric::CGauge& chaos::common::metric::CGauge::operator-=(const double d) {
    impl.Decrement(d);
    return *this;
}

#pragma mark CHistogram
chaos::common::metric::CHistogram::CHistogram(prometheus::Histogram& _impl):
impl(_impl){}

void chaos::common::metric::CHistogram::observe(double value) {
    impl.Observe(value);
}

#pragma mark Manager
MetricManager::MetricManager():
http_exposer(new prometheus::Exposer(GlobalConfiguration::getInstance()->getConfiguration()->getStringValue(InitOption::OPT_METRIC_WEB_SERVER_PORT))),
metrics_registry(std::make_shared<Registry>()),
io_send_byte_sec(BuildCounter()
                 .Name("io_tx_data")
                 .Help("The data rate of transmitted out of data service")
                 .Labels({})
                 .Register(*metrics_registry)),
io_send_packet_sec(BuildCounter()
                   .Name("io_tx_packet")
                   .Help("The data rate of transmitted packet out of data service")
                   .Labels({})
                   .Register(*metrics_registry)),
io_receive_byte_sec(BuildCounter()
                    .Name("io_rx_data")
                    .Help("The data rate of received out of data service")
                    .Labels({})
                    .Register(*metrics_registry)),
io_receive_packet_sec(BuildCounter()
                      .Name("io_rx_packet")
                      .Help("The data rate of received packet out of data service")
                      .Labels({})
                      .Register(*metrics_registry)) {
    http_exposer->RegisterCollectable(metrics_registry);
}

MetricManager::~MetricManager() {}

void MetricManager::init(void *data) {}

void MetricManager::deinit() {
    metrics_registry.reset();
    http_exposer.reset();
}

CounterUniquePtr MetricManager::getNewTxDataRateMetricFamily(const std::map<std::string,std::string>& label) {
    return CounterUniquePtr(new chaos::common::metric::CCounter(io_send_byte_sec.Add(label)));
}

CounterUniquePtr MetricManager::getNewRxDataRateMetricFamily(const std::map<std::string,std::string>& label) {
    return CounterUniquePtr(new chaos::common::metric::CCounter(io_receive_byte_sec.Add(label)));
}

CounterUniquePtr MetricManager::getNewTxPacketRateMetricFamily(const std::map<std::string,std::string>& label) {
    return CounterUniquePtr(new chaos::common::metric::CCounter(io_send_packet_sec.Add(label)));
}

CounterUniquePtr MetricManager::getNewRxPacketRateMetricFamily(const std::map<std::string,std::string>& label) {
    return CounterUniquePtr(new chaos::common::metric::CCounter(io_receive_packet_sec.Add(label)));
}

void MetricManager::createCounterFamily(const std::string& name,
                                        const std::string& desc) {
    LMapFamilyCounterWriteLock wl = map_counter.getWriteLockObject();
    
    //check if family already exists
    if(map_counter().find(name) != map_counter().end()) return;
    
    map_counter().insert(MapFamilyCounterPair(name,
                                              BuildCounter()
                                              .Name(name)
                                              .Help(desc)
                                              .Labels({})
                                              .Register(*metrics_registry)));
}

void MetricManager::createGaugeFamily(const std::string& name,
                                      const std::string& desc) {
    LMapFamilyGaugeWriteLock wl = map_gauge.getWriteLockObject();
    
    //check if family already exists
    if(map_gauge().find(name) != map_gauge().end()) return;
    
    map_gauge().insert(MapFamilyGaugePair(name,
                                          BuildGauge()
                                          .Name(name)
                                          .Help(desc)
                                          .Labels({})
                                          .Register(*metrics_registry)));
}

void MetricManager::createHistogramFamily(const std::string& name,
                                          const std::string& desc) {
    LMapFamilyHistogramWriteLock wl = map_histogram.getWriteLockObject();
    
    //check if family already exists
    if(map_histogram().find(name) != map_histogram().end()) return;
    
    map_histogram().insert(MapFamilyHistogramPair(name,
                                                  BuildHistogram()
                                                  .Name(name)
                                                  .Help(desc)
                                                  .Labels({})
                                                  .Register(*metrics_registry)));
}

CounterUniquePtr MetricManager::getNewCounterFromFamily(const std::string& family_name,
                                                        const std::map<std::string,std::string>& label) {
    LMapFamilyCounterReadLock wl = map_counter.getReadLockObject();
    MapFamilyCounterIterator i = map_counter().find(family_name);
    if(i == map_counter().end()) return CounterUniquePtr();
    return CounterUniquePtr(new chaos::common::metric::CCounter(i->second.Add(label)));
}

GaugeUniquePtr MetricManager::getNewGaugeFromFamily(const std::string& family_name,
                                                    const std::map<std::string,std::string>& label) {
    LMapFamilyGaugeReadLock wl = map_gauge.getReadLockObject();
    MapFamilyGaugeIterator i = map_gauge().find(family_name);
    if(i == map_gauge().end()) return GaugeUniquePtr();
    return GaugeUniquePtr(new chaos::common::metric::CGauge(i->second.Add(label)));
}

HistogramUniquePtr MetricManager::getNewHistogramFromFamily(const std::string& family_name,
                                                            const std::map<std::string,std::string>& label,
                                                            const CHistogramBoudaries& boundaries) {
    LMapFamilyHistogramReadLock wl = map_histogram.getReadLockObject();
    MapFamilyHistogramIterator i = map_histogram().find(family_name);
    if(i == map_histogram().end()) return HistogramUniquePtr();
    return HistogramUniquePtr(new chaos::common::metric::CHistogram(i->second.Add(label, boundaries)));
}
