#include "save_state.hpp"

#include <fstream>
#include <stdexcept>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/optional.hpp>
#include <boost/serialization/map.hpp>

void save_tree(const MTree& tree, Node root, const std::string& filename) {
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    int root_name = tree[root].name;
    oa << root_name << tree;
}

std::pair<MTree, Node> load_tree(const std::string& filename) {
    MTree tree;
    int root_name;
    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> root_name >> tree;

    for (auto vi = vertices(tree).first; vi != vertices(tree).second; ++vi) {
        if (tree[*vi].name == root_name) {
            return {std::move(tree), *vi};
        }
    }
    throw std::runtime_error("Root node not found in loaded tree");
}

void save_fg(const FG& fg, const FG_Map& fg_params, const FG_Map& fg_factors, const std::string& filename) {
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    oa << fg << fg_params << fg_factors;
}

std::tuple<FG, FG_Map, FG_Map> load_fg(const std::string& filename) {
    FG fg;
    FG_Map fg_params;
    FG_Map fg_factors;
    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> fg >> fg_params >> fg_factors;
    return {std::move(fg), std::move(fg_params), std::move(fg_factors)};
}

void save_state(const MTree& tree, Node root,
                const FG& fg, const FG_Map& fg_params, const FG_Map& fg_factors,
                const std::string& filename) {
    std::ofstream ofs(filename);
    boost::archive::text_oarchive oa(ofs);
    int root_name = tree[root].name;
    oa << root_name << tree << fg << fg_params << fg_factors;
}

std::tuple<MTree, Node, FG, FG_Map, FG_Map> load_state(const std::string& filename) {
    MTree tree;
    int root_name;
    FG fg;
    FG_Map fg_params;
    FG_Map fg_factors;

    std::ifstream ifs(filename);
    boost::archive::text_iarchive ia(ifs);
    ia >> root_name >> tree >> fg >> fg_params >> fg_factors;

    for (auto vi = vertices(tree).first; vi != vertices(tree).second; ++vi) {
        if (tree[*vi].name == root_name) {
            return {std::move(tree), *vi, std::move(fg), std::move(fg_params), std::move(fg_factors)};
        }
    }
    throw std::runtime_error("Root node not found in loaded state");
}
