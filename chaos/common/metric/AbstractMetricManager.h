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


#ifndef ChaosCommon_AbstractMetricManager_h
#define ChaosCommon_AbstractMetricManager_h

#include <chaos/common/chaos_types.h>
#include <chaos/common/utility/Singleton.h>
#include <chaos/common/utility/LockableObject.h>
#include <chaos/common/utility/InizializableService.h>

namespace chaos {
    namespace common {
        namespace metric {
            class MetricManager;
            
            /*!
             Counter abstraction
             */
            class CCounter {
                friend class MetricManager;
            public:
                virtual double operator()() = 0;
                virtual CCounter& operator++() = 0;
                virtual CCounter& operator++(int) = 0;
                virtual double operator+(const double d) const = 0;
                virtual CCounter& operator+=(const double d) = 0;
            };
            
            /*!
             Gauge abstraction
             */
            class CGauge {
                friend class MetricManager;
            public:
                virtual double operator()() = 0;
                virtual CGauge& operator--() = 0;
                virtual CGauge& operator++() = 0;
                virtual CGauge& operator--(int) = 0;
                virtual CGauge& operator++(int) = 0;
                virtual double operator+(const double d) const = 0;
                virtual double operator-(const double d) const = 0;
                virtual double operator=(const double d) const = 0;
                virtual CGauge& operator+=(const double d) = 0;
                virtual CGauge& operator-=(const double d) = 0;
            };
            
            typedef ChaosUniquePtr<CCounter> CounterUniquePtr;
            typedef ChaosUniquePtr<CGauge> GaugeUniquePtr;
            
            /*!
             Central managment for metric exposition to Prometheus
             */
            class AbstractMetricManager:
            public common::utility::InizializableService,
            public chaos::common::utility::Singleton<MetricManager> {
                friend class chaos::common::utility::Singleton<MetricManager>;
                
                AbstractMetricManager();
                ~AbstractMetricManager();
            protected:
                void init(void *data);
                void deinit();
            public:
                
                virtual CounterUniquePtr getNewTxDataRateMetricFamily(const std::map<std::string,std::string>& label) = 0;
                virtual CounterUniquePtr getNewRxDataRateMetricFamily(const std::map<std::string,std::string>& label) = 0;
                virtual CounterUniquePtr getNewTxPacketRateMetricFamily(const std::map<std::string,std::string>& label) = 0;
                virtual CounterUniquePtr getNewRxPacketRateMetricFamily(const std::map<std::string,std::string>& label) = 0;
                
                virtual void createCounterFamily(const std::string& name,
                                                 const std::string& desc) = 0;
                
                virtual void createGaugeFamily(const std::string& name,
                                               const std::string& desc) = 0;
                
                virtual CounterUniquePtr getNewCounterFromFamily(const std::string& family_name,
                                                                 const std::map<std::string,std::string>& label = {}) = 0;
                
                virtual GaugeUniquePtr getNewGaugeFromFamily(const std::string& family_name,
                                                             const std::map<std::string,std::string>& label = {}) = 0;
                
            };
        }
    }
}

#endif /*ChaosCommon_AbstractMetricManager_h */
