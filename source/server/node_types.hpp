//  node types
//  Copyright (C) 2008, 2009 Tim Blechmann
//
//  This program is free software; you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation; either version 2 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; see the file COPYING.  If not, write to
//  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
//  Boston, MA 02111-1307, USA.

#ifndef SERVER_NODE_TYPES_HPP
#define SERVER_NODE_TYPES_HPP

#include <boost/detail/atomic_count.hpp>
#include <boost/intrusive/list.hpp>
#include <boost/intrusive/set.hpp>
#include <boost/intrusive_ptr.hpp>

#include "synth_prototype.hpp"
#include "utilities/static_pool.hpp"

namespace nova
{

class server_node;

class abstract_group;
class group;

namespace bi = boost::intrusive;

class server_node:
    public bi::list_base_hook<bi::link_mode<bi::auto_unlink> >,
    public bi::set_base_hook<bi::link_mode<bi::auto_unlink> >  /* for node_id mapping */
{
protected:
    server_node(int node_id, bool type):
        node_id(node_id), synth_(type), parent_(0), use_count_(0)
    {}

    virtual ~server_node(void)
    {
        assert(parent_ == 0);
    }

public:
    typedef bi::list_base_hook<bi::link_mode<bi::auto_unlink> > parent_hook;

    /* @{ */
    /** node_id mapping */
    friend bool operator< (server_node const & lhs, server_node const & rhs)
    {
        return lhs.node_id < rhs.node_id;
    }
    friend bool operator== (server_node const & lhs, server_node const & rhs)
    {
        return lhs.node_id == rhs.node_id;
    }
    const int node_id;
    /* @} */

    bool is_synth(void) const
    {
        return synth_;
    }

    /** set a slot */
    /* @{ */
    virtual void set(slot_identifier_type const & slot_str, float val) = 0;
    virtual void set(slot_index_t slot_id, float val) = 0;
    /* @} */

private:
    bool synth_;

    friend class node_graph;
    friend class abstract_group;
    friend class group;
    friend class parallel_group;

    void set_parent(abstract_group * parent)
    {
        add_ref();
        assert(parent_ == 0);
        parent_ = parent;
    }

    void clear_parent(void)
    {
        parent_ = 0;
        release();
    }

    abstract_group * parent_;

public:
    /* memory management for server_nodes */
    /* @{ */
    static void * allocate(std::size_t size);
    static void free(void *);
    static std::size_t get_max_size(void)
    {
        return pool.get_max_size();
    }

private:
    typedef static_pool<1024*1024> node_pool;
    static node_pool pool;
    /* @} */

    /* call destructor and free to node_pool */
    void delete_this(void)
    {
        this->~server_node();
        server_node::free(this);
    }

    /* refcountable */
    /* @{ */
public:
    void add_ref(void)
    {
        ++use_count_;
    }

    void release(void)
    {
        if(--use_count_ == 0)
            delete_this();
    }

private:
    boost::detail::atomic_count use_count_;
    /* @} */
};

inline void intrusive_ptr_add_ref(server_node * p)
{
    p->add_ref();
}

inline void intrusive_ptr_release(server_node * p)
{
    p->release();
}

typedef boost::intrusive_ptr<server_node> server_node_ptr;
typedef boost::intrusive_ptr<class synth> synth_ptr;
typedef boost::intrusive_ptr<group> group_ptr;

enum node_position
{
    head,
    tail,
    before,
    after,
    replace,
    insert                      /* for pgroups */
};

typedef std::pair<server_node *, node_position> node_position_constraint;


template <typename synth_t>
inline synth_t * synth_prototype::allocate(void)
{
    return static_cast<synth_t*>(server_node::allocate(sizeof(synth_t)));
}



/* allocator class, using server_node specific memory pool */
template <class T>
class server_node_allocator
{
public:
    typedef std::size_t size_type;
    typedef std::ptrdiff_t difference_type;
    typedef T*        pointer;
    typedef const T*  const_pointer;
    typedef T&        reference;
    typedef const T&  const_reference;
    typedef T         value_type;

    template <class U> struct rebind
    {
        typedef server_node_allocator<U> other;
    };

    server_node_allocator(void) throw()
    {}

    ~server_node_allocator() throw()
    {}

    pointer address(reference x) const
    {
        return &x;
    }

    const_pointer address(const_reference x) const
    {
        return &x;
    }

    pointer allocate(size_type n,
                     const_pointer hint = 0)
    {
        pointer ret = static_cast<pointer>(server_node::allocate(n * sizeof(T)));
        if (ret == 0)
            throw std::bad_alloc();

        return ret;
    }

    void deallocate(pointer p, size_type n)
    {
        server_node::free(p);
    }

    size_type max_size() const throw()
    {
        return server_node::get_max_size();
    }

    void construct(pointer p, const T& val)
    {
        ::new(p) T(val);
    }

    void destroy(pointer p)
    {
        p->~T();
    }
};


template<typename T, typename U>
bool operator==( server_node_allocator<T> const& left, server_node_allocator<U> const& right )
{
    return !(left != right);
}

template<typename T, typename U>
bool operator!=( server_node_allocator<T> const& left, server_node_allocator<U> const& right )
{
    return true;
}


} /* namespace nova */

#endif /* SERVER_NODE_TYPES_HPP */
