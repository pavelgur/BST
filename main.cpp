#include <algorithm>
#include <cassert>
#include <deque>
#include <vector>
#include <unordered_set>


template<class TKey, class TValue, class TLess = std::less<TKey>, class TEqual = std::equal_to<TKey>>
class TTree {
    struct TNode {
        TNode(TKey key, TValue val)
            : Key(std::move(key))
            , Value(std::move(val))
        {}

        bool operator<(const TKey& k) const noexcept {
            return TLess()(Key, k);
        }

        bool operator==(const TKey& k) const noexcept {
            return TEqual()(Key, k);
        }

        size_t Left = 0;
        size_t Right = 0;
        TKey Key;
        TValue Value;
        bool Deleted = false;
    };

public:
    void Add(TKey key, TValue value) {
        if (Nodes.empty()) {
            Nodes.emplace_back(std::move(key), std::move(value));
            return;
        }

        for (size_t curNodeIdx = 0;;) {
            auto& curNode = Nodes[curNodeIdx];
            if (curNode < key) {
                if (curNode.Right) {
                    curNodeIdx = curNode.Right;
                } else {
                    curNode.Right = Nodes.size();
                    Nodes.emplace_back(std::move(key), std::move(value));
                    return;
                }
            } else if (curNode == key) {
                std::swap(curNode.Value, value);
                return;
            } else {
                if (curNode.Left) {
                    curNodeIdx = curNode.Left;
                } else {
                    curNode.Left = Nodes.size();
                    Nodes.emplace_back(std::move(key), std::move(value));
                    return;
                }
            }
        }
    }

    const TValue* FindPtr(const TKey& key) const noexcept {
        if (Nodes.empty()) {
            return nullptr;
        }

        for (size_t curNodeIdx = 0;;) {
            auto& curNode = Nodes[curNodeIdx];
            if (curNode < key) {
                if (curNode.Right) {
                    curNodeIdx = curNode.Right;
                    continue;
                }
                return nullptr;
            } else if (curNode == key) {
                return std::addressof(curNode.Value);
            } else {
                if (curNode.Left) {
                    curNodeIdx = curNode.Left;
                    continue;
                }
                return nullptr;
            }
        }
    }

    enum class ETraverseType {
        PreOrder,
        InOrder,
        PostOrder,
    };
    template<ETraverseType Type = ETraverseType::InOrder>
    void Traverse(std::vector<std::pair<TKey, TValue>>& res) const noexcept {
        res.clear();
        if (Nodes.empty()) {
            return;
        }
        TraverseInt<Type>(res);
    }

private:
    template<ETraverseType Type>
    void TraverseInt(std::vector<std::pair<TKey, TValue>>& res, size_t idx = 0) const noexcept {
        const TNode& node = Nodes[idx];

        const auto processMid = [&] () {
            res.emplace_back(node.Key, node.Value);
        };
        const auto processLeft = [&] () {
            if (node.Left) {
                TraverseInt<Type>(res, node.Left);
            }
        };
        const auto processRight = [&] () {
            if (node.Right) {
                TraverseInt<Type>(res, node.Right);
            }
        };

        switch (Type) {
        case ETraverseType::PreOrder:
            processMid();
            processLeft();
            processRight();

            break;

        case ETraverseType::InOrder:
            processLeft();
            processMid();
            processRight();

            break;

        case ETraverseType::PostOrder:
            processLeft();
            processRight();
            processMid();

            break;

        default:
            assert(false);
        }
    }

private:
    std::deque<TNode> Nodes;
};

int main() {
    srand(777);

    const auto size = 1000u;
    std::vector<std::pair<int, int>> vec;
    vec.reserve(size);

    TTree<int, int> tree;
    std::unordered_set<int> keys;
    for (auto i = 0u; i < vec.capacity(); ++i) {
        const auto key = rand();
        if (!keys.insert(key).second) {
            continue;
        }

        const auto value = rand();

        vec.emplace_back(key, value);
        tree.Add(key, value);
    }

    {
        decltype(vec) treeValues;
        treeValues.reserve(vec.size());

        std::sort(vec.begin(), vec.end(), [] (const auto& a, const auto& b) {
            return a.first < b.first;
        });
        tree.Traverse(treeValues);

        assert(std::equal_to<>()(vec, treeValues));
    }

    for (const auto& v : vec) {
        assert(tree.FindPtr(v.first));
        assert(!tree.FindPtr(-v.first));
    }

    return EXIT_SUCCESS;
}
