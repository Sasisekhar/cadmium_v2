/**
 * Root coordinator for real-time sequential simulation.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2021-present Román Cárdenas Rodríguez
 * ARSLab - Carleton University
 * GreenLSI - Polytechnic University of Madrid
 */

#ifndef CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_
#define CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_

#include <limits>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>
#include "root_coordinator.hpp"
#include "core/coordinator.hpp"
#include "rt_clock/rt_clock.hpp"

#include "external_output_handler.hpp"

namespace cadmium {
    /**
     * Real-time root coordinator.
     * @tparam T type of the real-time clock.
     */
    template <typename T, typename variantType = int>
    class RealTimeRootCoordinator: public RootCoordinator {
        static_assert(std::is_base_of<RealTimeClock, T>::value, "T must inherit cadmium::RealTimeClock");
     protected:
        T clock;  //!< Real-time clock.
        std::shared_ptr<outputHandler<variantType>> output_handle;
        bool external_output_enable;

        /**
         * The only difference with the sequential simulation is that we first wait until timeNext.
         * @param timeNext next simulation time (in seconds).
         */
        void simulationAdvance(double timeNext) override {
            double t = clock.waitUntil(timeNext);
            // double t = timeNext;
            // RootCoordinator::simulationAdvance(t);
            #ifndef NO_LOGGING
                if (logger != nullptr) {
                    logger->logTime(t);
                }
            #endif
            topCoordinator->collection(t);

            if(external_output_enable) {
                if(!topCoordinator->getComponent()->outEmpty()) {
                    auto topComponent = topCoordinator->getComponent();
                    auto outPorts = topComponent->getOutPorts();
                    std::vector<std::pair<variantType, std::string>> port_data;
                    for(auto& port : outPorts) {
                        Component outputComponent("OC");
                        Port<variantType> out;
                        out = outputComponent.addOutPort<variantType>("out");
                        out->propagate(port);
                        port_data.push_back(std::make_pair(out->getBag().back(), port->getId()));
                    }
                    output_handle->parseOutput(port_data);
                }
            }
            
            topCoordinator->transition(t);
            topCoordinator->clear();
        }

     public:
        RealTimeRootCoordinator(std::shared_ptr<Coupled> model, double time, T clock):
            RootCoordinator(model, time), clock(clock) {}

        RealTimeRootCoordinator(std::shared_ptr<Coupled> model, T clock): 
            RealTimeRootCoordinator(model, 0, clock) {
                external_output_enable = false;
            }

        RealTimeRootCoordinator(std::shared_ptr<Coupled> model, T clock, std::shared_ptr<outputHandler<variantType>> handler): 
            RealTimeRootCoordinator(model, 0, clock) {
                external_output_enable = true;
                this->output_handle = handler;
            }

        //! Starts the root coordinator and the real-time clock.
        void start() override {
            RootCoordinator::start();
            clock.start(topCoordinator->getTimeLast());
        }

        //! Stops the root coordinator and the real-time clock.
        void stop() override {
            clock.stop(topCoordinator->getTimeLast());
            RootCoordinator::stop();
        }

	    void simulate(double timeInterval) override {
            double timeNext = topCoordinator->getTimeNext();
            double timeFinal = topCoordinator->getTimeLast() + timeInterval;
            
            while(timeNext <= timeFinal/* || timeInterval == std::numeric_limits<double>::infinity()*/) {
                this->simulationAdvance(timeNext);
                timeNext = topCoordinator->getTimeNext();
            }
        }

    };
}

#endif // CADMIUM_SIMULATION_RT_ROOT_COORDINATOR_HPP_
