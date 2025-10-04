#ifndef BINDER_H
#define BINDER_H

#include <iostream>
#include <list>
#include <map>
#include <functional>
#include <memory>
#include <utility>
#include <iterator>

namespace cxx
{
    template <typename K, typename V>
    class binder
    {
    public:
        binder();
        binder(const binder& b);
        binder(binder && b);
        binder& operator=(binder b); // CO
        ~binder() = default;

        void insert_front(const K& k, const V& v);
        void insert_after(const K& prev_k, const K& k, const V& v);

        void remove();
        void remove(const K& k);

        V& read(const K& k);
        const V& read(const K& k) const;

        size_t size() const;

        void clear();

        class const_iterator
        {
        private:
            using base_it_t = typename std::list<std::pair<K, V>>::const_iterator;
            base_it_t m_it;
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = V;
            using difference_type = std::ptrdiff_t;
            using pointer = const V*;
            using reference = const V&;

            const_iterator() = default;
            const_iterator(base_it_t it) : m_it{it} {}

            reference operator*() const { return m_it->second; }
            pointer operator->() const { return std::addressof(m_it->second); }

            const_iterator& operator++() { ++m_it; return *this; }
            const_iterator operator++(int) { auto tmp { *this }; ++m_it; return tmp; }

            bool operator==(const const_iterator& o) const { return m_it == o.m_it; }
            bool operator!=(const const_iterator& o) const { return m_it != o.m_it; }
        };

        const_iterator cbegin() const { return const_iterator(m_state->notes.cbegin()); }
        const_iterator cend() const { return const_iterator(m_state->notes.cend()); }
    private:
        struct KeyRefLess
        {
            using is_transparent = void; // enables heterogeneous lookup
            bool operator()(std::reference_wrapper<const K> a, std::reference_wrapper<const K> b) const 
            { return a.get() < b.get(); }
            bool operator()(std::reference_wrapper<const K> a, const K& b) const 
            { return a.get() < b; }
            bool operator()(const K& a, std::reference_wrapper<const K> b) const 
            { return a < b.get(); }
        };

        struct State
        {
            size_t size{};
            std::list<std::pair<K, V>> notes{};
            std::map<std::reference_wrapper<const K>, typename std::list<std::pair<K, V>>::iterator, KeyRefLess> finder{};
        };

        void remove_content(const std::shared_ptr<State>& state, const K& k);
        void insert_at_iterator(const std::shared_ptr<State>& state, std::list<std::pair<K, V>>::iterator it, const K&k, const V& v);

        std::shared_ptr<State> m_state{};

        std::shared_ptr<State> copy_on_write();

        inline static std::shared_ptr<State> s_empty { std::make_shared<State>() };
    };

    template <typename K, typename V>
    binder<K, V>::binder()
        : m_state{ std::make_shared<State>() }
    {}
    
    template <typename K, typename V>
    std::shared_ptr<typename binder<K,V>::State> binder<K, V>::copy_on_write()
    {
        if(!m_state)
            return std::make_shared<State>();

        constexpr static int singlePointing { 1 };

        if(m_state.use_count() == singlePointing) return nullptr;

        // deep copy 
        auto new_state { std::make_shared<State>() };
        new_state->size = m_state->size;
        
        // We can only assume copy-ctor in V and destructor IMPORTANT!!
        for (auto const &p : m_state->notes)
            new_state->notes.emplace_back(p.first, p.second);

        new_state->finder.clear();
        for (auto it = new_state->notes.begin(); it != new_state->notes.end(); ++it)
            new_state->finder.emplace(std::cref(it->first), it);
        
        return new_state;
    }

    template <typename K, typename V>
    binder<K, V>::binder(const binder& b) : 
        m_state{ b.m_state }
    {}

    template <typename K, typename V>
    binder<K, V>::binder(binder&& b) : 
        m_state{ std::move(b.m_state) }
    {}

    template <typename K, typename V>
    binder<K, V>& binder<K, V>::operator=(binder b)
    {
        std::swap(m_state, b.m_state);
        return *this;
    }

    template <typename K, typename V>
    void binder<K, V>::insert_at_iterator(const std::shared_ptr<State>& state, std::list<std::pair<K, V>>::iterator it, const K& k, const V& v)
    {
        auto new_it { state->notes.emplace(it, k, v) };
        try
        {
            state->finder.emplace(std::cref(new_it->first), new_it);
            ++state->size;
        }
        catch(...) 
        {
            state->notes.erase(new_it);
            throw;
        }
    }

    template <typename K, typename V>
    void binder<K, V>::insert_front(const K& k, const V& v)
    {
        auto it { m_state->finder.find(k) };
        if(it != m_state->finder.end())
            throw std::invalid_argument("insert_front: key already exists");

        auto new_state { copy_on_write() };

        if(new_state)
        {
            insert_at_iterator(new_state, new_state->notes.begin(), k, v);
            m_state = std::move(new_state); // commit
        } 
        else
        {
            insert_at_iterator(m_state, m_state->notes.begin(), k, v);
        }
    }

    template <typename K, typename V>
    void binder<K, V>::insert_after(const K& prev_k, const K& k, const V& v)
    {
        auto it_k { m_state->finder.find(k) };
        auto it_prev { m_state->finder.find(prev_k) };

        if(it_k != m_state->finder.end() || it_prev == m_state->finder.end() )
            throw std::invalid_argument("insert_after: invalid previous key or key already exists");
        
        auto new_state { copy_on_write() };
        
        if (new_state) 
        {
            insert_at_iterator(new_state, std::next(new_state->finder.find(prev_k)->second), k, v);
            m_state = std::move(new_state); // commit
        }
        else 
        {
            insert_at_iterator(m_state, std::next(m_state->finder.find(prev_k)->second), k, v);
        }
    }

    template <typename K, typename V>
    void binder<K, V>::remove()
    {
        if(size() == 0)
            throw std::invalid_argument("remove: container is empty");

        remove(m_state->notes.begin()->first);
    }

    template <typename K, typename V>
    void binder<K, V>::remove_content(const std::shared_ptr<State>& state, const K& k){
        auto it { state->finder.find(k) };
        state->notes.erase(it->second);
        state->finder.erase(it);
        --state->size;
    }

    template <typename K, typename V>
    void binder<K, V>::remove(const K& k)
    {
        if(size() == 0 || m_state->finder.find(k) == m_state->finder.end())
            throw std::invalid_argument("remove: key not found or container is empty");

        auto new_state { copy_on_write() };

        if (new_state) 
        {
            remove_content(new_state, k);
            m_state = std::move(new_state);
        }
        else
        {
            remove_content(m_state, k);
        }
    }

    template <typename K, typename V>
    V& binder<K, V>::read(const K& k)
    {   
        if(m_state->finder.find(k) == m_state->finder.end())
            throw std::invalid_argument("read: key not found");

        auto new_state { copy_on_write() };

        if (new_state) 
        {
            // find element in the copy
            auto it_copy { new_state->finder.find(k) };
            m_state = std::move(new_state);
            return it_copy->second->second;
        } 
        else 
        {
            auto it { m_state->finder.find(k) };
            return it->second->second;
        }
    }

    template <typename K, typename V>
    const V& binder<K, V>::read(const K& k) const
    {
        auto it = m_state->finder.find(k);
        if (it == m_state->finder.end())
            throw std::invalid_argument("read: key not found");
        return it->second->second;
    }

    template <typename K, typename V>
    size_t binder<K, V>::size() const { return m_state->size; }

    template <typename K, typename V>
    void binder<K, V>::clear()
    {
        if(!m_state)
        {
            m_state = s_empty;
            return;
        }

        if (m_state.use_count() == 1) 
        {
            m_state->notes.clear();
            m_state->finder.clear();
            m_state->size = 0;
            return;
        }

        m_state = s_empty;
    }

}

#endif
