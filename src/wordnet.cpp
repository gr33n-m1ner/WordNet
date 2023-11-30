#include "wordnet.h"

#include <iostream>
#include <iterator>
#include <queue>
#include <sstream>
#include <unordered_map>

// WordNet

WordNet::WordNet(std::istream & synsets, std::istream & hypernyms)
    : digraph(Digraph())
{

    std::string string_id, synonyms, definition;
    std::set<std::string> words;
    while (getline(synsets, string_id, ',')) {
        unsigned id = std::stoi(string_id);
        getline(synsets, synonyms, ',');
        getline(synsets, definition, '\n');
        definitions.emplace(id, definition);

        std::stringstream synstream = std::stringstream(synonyms);
        std::string synonym;
        while (synstream >> synonym) {
            words_id[synonym].emplace(id);
            words.emplace(synonym);
        }
    }

    std::string current_string;
    while (getline(hypernyms, current_string)) {
        if (current_string.empty()) {
            continue;
        }
        std::stringstream hypstream = std::stringstream(current_string);
        getline(hypstream, string_id, ',');
        unsigned id = std::stoi(string_id);
        std::set<unsigned> children_ids;
        while (getline(hypstream, string_id, ',')) {
            children_ids.emplace(std::stoi(string_id));
        }
        digraph.add_arc(id, children_ids);
    }
}

WordNet::Nouns WordNet::nouns() const
{
    return Nouns(words_id);
}

bool WordNet::is_noun(const std::string & word) const
{
    return words_id.find(word) != words_id.end();
}

std::string WordNet::sca(const std::string & noun1, const std::string & noun2) const
{
    ShortestCommonAncestor sca = ShortestCommonAncestor(digraph);
    return definitions.at(sca.ancestor_subset(words_id.at(noun1), words_id.at(noun2)));
}

unsigned WordNet::distance(const std::string & noun1, const std::string & noun2) const
{
    ShortestCommonAncestor sca = ShortestCommonAncestor(digraph);
    return sca.length_subset(words_id.at(noun1), words_id.at(noun2));
}

// Outcast

Outcast::Outcast(const WordNet & wordnet)
    : word_net(wordnet)
{
}

std::string Outcast::outcast(const std::set<std::string> & nouns)
{
    std::map<std::string, unsigned> distances;
    for (auto i = nouns.begin(); i != nouns.end(); ++i) {
        for (auto j = nouns.begin(); j != nouns.end(); ++j) {
            if (i == j) {
                continue;
            }
            else {
                distances[*i] += word_net.distance(*i, *j);
            }
        }
    }
    auto answer = distances.begin();
    unsigned count_max = 0;
    for (auto i = distances.begin(); i != distances.end(); ++i) {
        if (i->second > answer->second) {
            count_max = 1;
            answer = i;
        }
        else if (i->second == answer->second) {
            count_max++;
        }
    }

    if (count_max == 1) {
        return answer->first;
    }
    else {
        return "";
    }
}

// Digraph

void Digraph::add_arc(unsigned id, const std::set<unsigned> & children_ids)
{
    arcs.emplace(id, children_ids);
}

const std::set<unsigned> & Digraph::get_children_id(unsigned id) const
{
    return arcs.at(id);
}

std::ostream & operator<<(std::ostream & output, const Digraph & digraph)
{
    for (const auto & [id, children_ids] : digraph.arcs) {
        output << id << ": ";
        for (const auto child_id : children_ids) {
            output << child_id << " ";
        }
        output << "\n";
    }
    return output;
}

bool Digraph::contains(unsigned id) const
{
    return arcs.find(id) != arcs.end();
}

// ShortestCommonAncestor

ShortestCommonAncestor::ShortestCommonAncestor(const Digraph & dg)
    : digraph(dg)
{
}

unsigned ShortestCommonAncestor::length(unsigned v, unsigned w)
{
    return ancestral_path_solving({v}, {w}).second;
}

unsigned ShortestCommonAncestor::ancestor(unsigned v, unsigned w)
{
    return ancestral_path_solving({v}, {w}).first;
}

unsigned ShortestCommonAncestor::length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b)
{
    return ancestral_path_solving(subset_a, subset_b).second;
}

unsigned ShortestCommonAncestor::ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b)
{
    return ancestral_path_solving(subset_a, subset_b).first;
}

std::pair<unsigned, unsigned> ShortestCommonAncestor::ancestral_path_solving(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b)
{
    using Id = unsigned;
    using Distance = unsigned;
    enum class Letter
    {
        A,
        B
    };

    std::queue<Id> bfs_queue;
    std::unordered_map<Id, std::pair<Distance, Letter>> bfs;

    for (const auto id : subset_a) {
        bfs_queue.push(id);
        bfs.emplace(id, std::make_pair(0, Letter::A));
    }
    for (const auto id : subset_b) {
        auto [value, success] = bfs.emplace(id, std::make_pair(0, Letter::B));
        if (!success) {
            return {id, 0};
        }
        bfs_queue.push(id);
    }

    std::pair<Id, Distance> answer = {-1, -1};
    while (!bfs_queue.empty()) {
        const Id id = bfs_queue.front();
        bfs_queue.pop();
        if (!digraph.contains(id)) {
            continue;
        }
        auto [depth, letter] = bfs[id];

        for (const Id parent_id : digraph.get_children_id(id)) {
            if (bfs.find(parent_id) == bfs.end()) {
                bfs.emplace(parent_id, std::make_pair(depth + 1, letter));
                bfs_queue.push(parent_id);
                continue;
            }
            auto [parent_depth, parent_letter] = bfs[parent_id];
            if (parent_letter != letter && answer.second > parent_depth + depth + 1) {
                answer = {parent_id, parent_depth + depth + 1};
            }
        }
    }
    return answer;
}
