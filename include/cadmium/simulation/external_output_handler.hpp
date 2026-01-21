/**
 * Interrupt Component Handler for the real time clock.
 * SPDX-License-Identifier: MIT
 * Copyright (c) 2025-present Sasisekhar Mangalam Govind
 * ARSLab - Carleton University
 */

#ifndef EXTERNAL_OUTPUT_HANDLER_HPP
#define EXTERNAL_OUTPUT_HANDLER_HPP

namespace cadmium {
    
    template<typename variantType>
    class outputHandler {
        
        public:
        /**
         * The output handler abstract class. Override this class
         * to enable asynchronous outputs in your model
         */
        outputHandler(){};
        
        /**
         * This method must be overriden to obtain the value of the output of your top model,
         * and parse it.
         * 
         * @param a vector of pairs. Each pair has a variant type (type of your port), and port id
         * @return nothing. TODO! maybe return error codes to help with simulation?
         */
        virtual void parseOutput(std::vector<std::pair<variantType, std::string>> outputs) = 0;
    };
}

#endif //EXTERNAL_OUTPUT_HANDLER_HPP