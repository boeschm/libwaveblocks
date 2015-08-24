#ifndef WAVEBLOCKS_IO_YAML_SHAPE_LOADER_HPP
#define WAVEBLOCKS_IO_YAML_SHAPE_LOADER_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <array>

#include <yaml-cpp/yaml.h>

#include "../shape_commons.hpp"

namespace waveblocks
{
namespace yaml
{
    template<dim_t D>
    struct ShapeDecoder
    {
        AbstractShape<D>* operator()(YAML::Node const& config)
        {
            if (config["dimensionality"].as<int>() != D) {
                throw std::runtime_error("encountered shape dimensionality != requested shape dimensionality");
            }
            
            std::string type = config["type"].as<std::string>();
            
            if (type == "hyperbolic") {
                int sparsity = config["sparsity"].as<int>();
                
                if (config["limits"]) {
                    return new LimitedHyperbolicCutShape<D>(sparsity, decode_limits(config["limits"]));
                }
                else {
                    return new HyperbolicCutShape<D>(sparsity);
                }
            }
            else if (type == "hypercubic") {
                return new HyperCubicShape<D>(decode_limits(config["limits"]));
            }
            
            throw std::runtime_error("unknown shape type");
        }
        
    private:
        std::array<int,D> decode_limits(YAML::Node const& node)
        {
            std::array<int,D> limits;
            
            if (node.size() != D) {
                throw std::runtime_error("size of limits != shape dimensionality");
            }
            
            for (std::size_t i = 0; i < node.size(); i++) {
                limits[i] = node[i].as<int>();
            }
            
            return limits;
        }
    };
}

}

#endif