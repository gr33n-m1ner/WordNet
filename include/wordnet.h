#include <iosfwd>
#include <map>
#include <set>
#include <string>

class Digraph
{
    std::map<unsigned, std::set<unsigned>> arcs;

public:
    void add_arc(unsigned id, const std::set<unsigned> & child_id);

    const std::set<unsigned> & get_children_id(unsigned id) const;

    bool contains(unsigned id) const;

    friend std::ostream & operator<<(std::ostream & output, const Digraph & digraph);
};

class WordNet
{
public:
    WordNet(std::istream & synsets, std::istream & hypernyms);

    class Nouns
    {
        using nouns_type = std::map<std::string, std::set<unsigned>>;
        const nouns_type & words;

    public:
        class iterator
        {
            friend class Nouns;
            nouns_type::const_iterator pos;
            iterator(nouns_type::const_iterator position)
                : pos(position){};

        public:
            using difference_type = std::ptrdiff_t;
            using value_type = std::string;
            using pointer = const value_type *;
            using reference = const value_type &;
            using iterator_category = std::forward_iterator_tag;

            iterator() = default;

            friend bool operator==(const iterator & left, const iterator & right)
            {
                return left.pos == right.pos;
            }

            friend bool operator!=(const iterator & left, const iterator & right)
            {
                return !(left == right);
            }

            reference operator*() const
            {
                return (*pos).first;
            }

            pointer operator->() const
            {
                return &((*pos).first);
            }

            iterator & operator++()
            {
                ++pos;
                return *this;
            }

            iterator operator++(int)
            {
                auto tmp = *this;
                operator++();
                return tmp;
            }
        };

        explicit Nouns(const nouns_type & words_ids)
            : words(words_ids){};

        iterator begin() const
        {
            return iterator(words.begin());
        }
        iterator end() const
        {
            return iterator(words.end());
        }
    };

    Nouns nouns() const;

    bool is_noun(const std::string & word) const;

    std::string sca(const std::string & noun1, const std::string & noun2) const;

    unsigned distance(const std::string & noun1, const std::string & noun2) const;

private:
    std::map<unsigned, std::string> definitions;
    std::map<std::string, std::set<unsigned>> words_id;
    Digraph digraph;
};

class Outcast
{
    WordNet word_net;

public:
    explicit Outcast(const WordNet & wordnet);

    std::string outcast(const std::set<std::string> & nouns);
};

class ShortestCommonAncestor
{
    const Digraph & digraph;

    std::pair<unsigned, unsigned> ancestral_path_solving(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b);

public:
    explicit ShortestCommonAncestor(const Digraph & dg);

    unsigned length(unsigned v, unsigned w);

    unsigned ancestor(unsigned v, unsigned w);

    unsigned length_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b);

    unsigned ancestor_subset(const std::set<unsigned> & subset_a, const std::set<unsigned> & subset_b);
};
