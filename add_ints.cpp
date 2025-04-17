#include <iostream>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <cassert>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <type_traits>

// Include the interval_map implementation
template <typename K, typename V>
class interval_map
{
    friend void IntervalMapTest(); // Friend function for testing
    V m_valBegin;                  // Initial value for the map
    std::map<K, V> m_map;          // The map storing intervals

public:
    // Constructor that associates the entire range of K with the value val
    template <typename V_forward>
    interval_map(V_forward &&val)
        : m_valBegin(std::forward<V_forward>(val)) {}

    // Assign the value val to the interval [keyBegin, keyEnd)
    template <typename V_forward>
    void assign(const K &keyBegin, const K &keyEnd, V_forward &&val)
    // requires std::is_same_v<std::remove_cvref_t<V_forward>, V>
    {
        // std::cout << "Assigning: " << val << "[" << keyBegin << ":" << keyEnd << "]" << std::endl;

        // If the interval is invalid, return
        if (!(keyBegin < keyEnd))
        {
            // this->print();
            return;
        }

        // If the map is empty, insert the first interval
        if (m_map.empty())
        {
            if (val != m_valBegin)
            {
                m_map.insert_or_assign(keyEnd, m_valBegin);
                m_map.insert_or_assign(keyBegin, std::forward<V_forward>(val));
                // this->print();
            }
            return;
        }

        // If the interval already exists with the same value, return
        auto begin_val = operator[](keyBegin);
        if (begin_val == val)
        {
            bool needsChange = false;
            auto it = m_map.lower_bound(keyBegin);
            auto end = m_map.lower_bound(keyEnd);

            // Check if there are any different values in the range
            while (it != end)
            {
                if (!(it->second == val))
                {
                    needsChange = true;
                    break;
                }
                ++it;
            }

            if (!needsChange)
            {
                // this->print();
                return;
            }
        }

        K last_key = (m_map.rbegin()->first < keyEnd) ? keyEnd : m_map.rbegin()->first;
        {
            std::pair<K, V> *resized = nullptr;
            std::vector<typename std::map<K, V>::iterator> will_delete;

            // Find the first element whose key is not less than keyEnd
            auto itBegin = m_map.lower_bound(keyBegin);
            auto itEnd = m_map.lower_bound(keyEnd);
            auto iterator_end = (itEnd == m_map.end() ? itEnd : std::next(itEnd));

            // Iterate over the range and delete overlapping records
            for (auto it = itBegin; it != iterator_end; ++it)
            {
                // existing interval is inside new
                // started location as new
                // if (it->first == keyBegin && it->second == val)
                if (!(keyBegin < it->first) && !(it->first < keyBegin) && it->second == val)
                {
                    auto next_it = std::next(it);
                    if (next_it != m_map.end() && next_it->second == val)
                    {
                        will_delete.push_back(next_it);
                    }
                }
                // started location inside new
                else if (!(keyEnd < it->first) && it->second == val)
                {
                    will_delete.push_back(it);
                }
                else
                {
                    // if the interval hits the new boundary
                    // if (it->first >= keyBegin && it->first < keyEnd)
                    if (!(it->first < keyBegin) && it->first < keyEnd)
                    {
                        auto next_it = std::next(it);
                        if (next_it != m_map.end() && keyEnd < next_it->first)
                        {
                            resized = new std::pair(keyEnd, it->second);
                        }
                        will_delete.push_back(it);
                    }
                }
            }

            // Insert resized interval if needed
            if (resized != nullptr)
            {
                m_map.insert_or_assign(resized->first, std::forward<V_forward>(resized->second));
            }

            // Delete overlapping records
            for (auto it : will_delete)
            {
                m_map.erase(it);
            }
        }

        // Insert the new interval
        {
            auto insert = m_map.insert_or_assign(keyBegin, std::forward<V_forward>(val)).first;
            if (insert != m_map.end())
            {
                auto what_after = std::next(insert);
                // Looking for same sequential intervals left
                if (what_after->second == val)
                {
                    m_map.erase(what_after);
                }
            }

            if (insert != m_map.begin())
            {
                auto what_before = std::prev(insert);
                // Looking for same sequential intervals right
                if (what_before->second == val)
                {
                    m_map.erase(insert);
                }
            }
        }

        // Canonization: remove valBegin from the map top
        while (!m_map.empty() && m_map.begin()->second == m_valBegin)
        {
            m_map.erase(m_map.begin());
        }

        // Remove only m_valBegin sequential intervals
        if (!m_map.empty())
        {
            auto last_map_item = std::prev(m_map.end());
            if (last_map_item->second == m_valBegin)
            {
                if (m_map.size() > 1)
                {
                    auto last_map_item_1 = std::prev(last_map_item);
                    if (last_map_item_1->second == m_valBegin)
                    {
                        // If the full map is m_valBegin
                        if (m_map.size() == 2)
                            m_map.erase(last_map_item_1);
                        m_map.erase(last_map_item);
                    }
                }
                else
                {
                    // Remove only one m_valBegin interval
                    m_map.erase(last_map_item);
                }
            }
        }

        // If the map is empty, return
        if (m_map.empty())
        {
            // this->print();
            return;
        }

        // If the last value in the map is not m_valBegin, set it
        if (!(m_map.rbegin()->second == m_valBegin))
        {
            m_map.insert_or_assign(keyEnd, m_valBegin);
        }

        // this->print();
    }

    // Look up the value associated with a key
    const V &operator[](const K &key) const
    {
        auto it = m_map.upper_bound(key);
        if (it == m_map.begin())
        {
            return m_valBegin;
        }
        else
        {
            return (--it)->second;
        }
    }

    // Check if the internal representation is canonical
    bool is_canonical() const
    {
        if (m_map.empty())
            return true;

        if (m_map.begin()->second == m_valBegin)
        {
            return false;
        }

        auto it = m_map.begin();
        auto prev_value = it->second;
        ++it;

        while (it != m_map.end())
        {
            if (it->second == prev_value)
            {
                return false;
            }
            prev_value = it->second;
            ++it;
        }

        return true;
    }

    // Get the internal map size
    size_t size() const
    {
        return m_map.size();
    }

    // Print the map contents
    void print() const
    {
        std::cout << "m_valBegin: " << m_valBegin << std::endl;
        std::cout << "Map contents:" << std::endl;
        for (const auto &[key, value] : m_map)
        {
            std::cout << key << " -> " << value << std::endl;
        }
        std::cout << std::endl;
    }

    // Get the initial value of the map
    const V get_valBegin()
    {
        return m_valBegin;
    }
};

interval_map<int, char> m('A');

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{

    int a, b;
    char c;

    if (size >= 9)
    {
        memcpy(&a, data, 4);
        memcpy(&b, data + 4, 4);
        memcpy(&c, data + 8, 1);

        m.assign(a, b, c);
    }

    return 0;
}

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        // Normal execution (if not being fuzzed)
        int a, b;
        char c;

        std::cout << "Enter two integers and char: ";
        std::cin >> a >> b >> c;

        m.assign(a, b, c);
        std::cout << "Is canonical: " << m.is_canonical() << std::endl;
        std::cout << "Size: " << m.size() << std::endl;

        // m.print();
        return 0;
    }
    else
    {
        // Fuzzing mode, will not reach here.
        return 0;
    }
}