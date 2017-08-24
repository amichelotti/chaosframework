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

#include <chaos_micro_unit_toolkit/connection/ConnectionManager.h>

//protocols
#include <chaos_micro_unit_toolkit/connection/protocol_adapter/http/HTTPProtocolAdapter.h>

//unit
#include <chaos_micro_unit_toolkit/connection/unit_proxy/raw_driver/RawDriverUnitProxy.h>

using namespace chaos::micro_unit_toolkit;
using namespace chaos::micro_unit_toolkit::connection;
using namespace chaos::micro_unit_toolkit::connection::unit_proxy;
using namespace chaos::micro_unit_toolkit::connection::protocol_adapter;

ConnectionManager::ConnectionManager() {
    registerUnitProxy<raw_driver::RawDriverUnitProxy>();
    registerProtocolAdapter<http::HTTPProtocolAdapter>();
}

ConnectionManager::~ConnectionManager() {}

ChaosUniquePtr<AbstractProtocolAdapter> ConnectionManager::getProtocolAdapter(ProtocolType type,
                                                                              const std::string& endpoint,
                                                                              const std::string& protocol_option) {
    if(map_protocol.count(type) == 0){ return ChaosUniquePtr<AbstractProtocolAdapter>();}
    ProtocolAdapterInstancer is = map_protocol[type];
    return ChaosUniquePtr<protocol_adapter::AbstractProtocolAdapter>(is->getInstance(endpoint,
                                                                                     protocol_option));
}

ChaosUniquePtr<AbstractUnitProxy> ConnectionManager::getUnitProxy(ProxyType type,
                                                                  ChaosUniquePtr<protocol_adapter::AbstractProtocolAdapter>& protocol_adapter){
    if(map_proxy.count(type) == 0){ return ChaosUniquePtr<unit_proxy::AbstractUnitProxy>();}
    UnitProxyInstancer is = map_proxy[type];
    return ChaosUniquePtr<unit_proxy::AbstractUnitProxy>(is->getInstance(protocol_adapter));
}
