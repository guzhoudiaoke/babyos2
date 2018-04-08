/*
 * guzhoudiaoke@126.com
 * 2017-12-24
 */

#ifndef _LIST_H_
#define _LIST_H_

#include "types.h"
#include "spinlock.h"
#include "pool.h"

template<typename T>
class list_node_t {
public:
    void init(const T& data) {
        m_data = data;
        m_prev = NULL;
        m_next = NULL;
    }

public:
    T            m_data;
	list_node_t* m_next;
    list_node_t* m_prev;
};

template<typename T>
class list_t {
public:
    class iterator {
        friend class list_t;

    public:
        iterator() {
            m_ptr = NULL;
        }
        iterator(list_node_t<T>* ptr) {
            m_ptr = ptr;
        }

        iterator& operator ++() {
            m_ptr = m_ptr->m_next;
            return *this;
        }
        iterator& operator --() {
            m_ptr = m_ptr->m_prev;
        }
        iterator& operator ++(int) {
            list_t::iterator it(m_ptr);
            m_ptr = m_ptr->m_next;
            return it;
        }
        iterator& operator --(int) {
            list_t::iterator it(m_ptr);
            m_ptr = m_ptr->m_prev;
            return it;
        }
        bool operator ==(const iterator &it) {
            return m_ptr == it.m_ptr;
        }
        bool operator !=(const iterator &it) {
            return m_ptr != it.m_ptr;
        }
        iterator& operator = (iterator it) {
            m_ptr = it.m_ptr;
            return *this;
        }
        T& operator * () {
            return m_ptr->m_data;
        }

    private:
        list_node_t<T>* m_ptr;
    };

    list_t<T>() {
        m_head = NULL;
        m_tail = NULL;
        m_pool = NULL;
        m_size = 0;
    }

    ~list_t<T>() {
        list_node_t<T>* p = m_head;
        while (p != NULL) {
            list_node_t<T>* d = p;
            p = p->m_next;
            free_node(d);
        }
        m_head = m_tail = NULL;
        m_size = 0;
    }

    void init(object_pool_t* pools) {
        m_head = NULL;
        m_tail = NULL;
        m_size = 0;

        uint32 node_size = sizeof(list_node_t<T>);
        if (node_size > SMALL_POOL_SIZE) {
            m_big_pool.init(node_size);
            m_pool = &m_big_pool;
        }
        else {
            m_pool = &pools[node_size];
        }
        m_lock.init();
    }

    list_t<T>& operator = (const list_t<T>& list) {
        m_head = NULL;
        m_tail = NULL;
        m_size = 0;

        uint32 node_size = sizeof(list_node_t<T>);
        if (node_size > SMALL_POOL_SIZE) {
            m_big_pool.init(node_size);
            m_pool = &m_big_pool;
        }
        else {
            m_pool = list.m_pool;
        }
        m_lock.init();

        const list_node_t<T>* p = list.m_head;
        while (p != NULL) {
            push_back(p->m_data);
            p = p->m_next;
        }
    }

    list_node_t<T>* alloc_node(const T& data) {
        list_node_t<T>* node = (list_node_t<T> *) m_pool->alloc_from_pool();
        node->init(data);
        return node;
    }

    void free_node(list_node_t<T>* node) {
        m_pool->free_object((void *) node);
    }

    bool empty() {
        return m_size == 0;
    }

    bool push_front(const T& data) {
        list_node_t<T>* node = alloc_node(data);
        if (node == NULL) {
            return false;
        }
        if (m_head == NULL && m_tail == NULL) {
            m_head = m_tail = node;
        }
        else {
            node->m_next = m_head;
            m_head->m_prev = node;
            m_head = node;
        }
        m_size++;
        return true;
    }

    bool push_back(const T& data) {
        list_node_t<T>* node = alloc_node(data);
        if (node == NULL) {
            return false;
        }
        if (m_head == NULL && m_tail == NULL) {
            m_head = m_tail = node;
        }
        else {
            m_tail->m_next = node;
            node->m_prev = m_tail;
            m_tail = node;
        }
        m_size++;
        return true;
    }

    bool pop_back() {
        if (m_size == 0) {
            return false;
        }
        list_node_t<T> *del = m_tail;
        m_size--;
        if (m_size == 0) {
            m_head = m_tail = NULL;
        }
        else {
            m_tail = m_tail->m_prev;
            m_tail->m_next = NULL;
        }
        free_node(del);
        return true;
    }

    bool pop_front() {
        if (m_size == 0) {
            return false;
        }

        list_node_t<T> *del = m_head;
        m_size--;
        if (m_size == 0) {
            m_head = m_tail = NULL;
        }
        else {
            m_head = m_head->m_next;
            m_head->m_prev = NULL;
        }
        free_node(del);
        return true;
    }

    list_t<T>::iterator insert(list_t<T>::iterator &it, const T& data) {
        /* insert before end, just push back */
        if (it.m_ptr == NULL) {
            push_back(data);
            return list_t::iterator(m_tail);
        }

        /* insert before front */
        if (it.m_ptr->m_prev == NULL) {
            push_front(data);
            return list_t::iterator(m_head);
        }

        /* insert middle */
        list_node_t<T>* node = alloc_node(data);
        if (node == NULL) {
            return list_t::iterator(NULL);
        }
        
        node->m_prev = it.m_ptr->m_prev;
        node->m_next = it.m_ptr;
        it.m_ptr->m_prev->m_next = node;
        it.m_ptr->m_prev = node;

        m_size++;
        return list_t::iterator(node);
    }

    list_t<T>::iterator erase(list_t<T>::iterator &it) {
        list_t<T>::iterator ret = it;

        /* error erase */
        if (m_size == 0) {
            return ret;
        }

        m_size--;

        /* no node left */
        if (m_size == 0) {
            m_head = m_tail = NULL;
            free_node(it.m_ptr);
            return list_t::iterator(NULL);
        }

        /* erase head */
        if (m_head == it.m_ptr) {
            m_head = m_head->m_next;
            free_node(it.m_ptr);
            return list_t::iterator(m_head);
        }

        /* erase tail */
        if (m_tail == it.m_ptr) {
            m_tail = m_tail->m_prev;
            m_tail->m_next = NULL;
            free_node(it.m_ptr);
            return list_t::iterator(m_tail);
        }

        /* erase middle node */
        if (it.m_ptr->m_prev != NULL) {
            it.m_ptr->m_prev->m_next = it.m_ptr->m_next;
        }
        if (it.m_ptr->m_next != NULL) {
            it.m_ptr->m_next->m_prev = it.m_ptr->m_prev;
        }

        ret.m_ptr = it.m_ptr->m_next;
        free_node(it.m_ptr);

        return ret;
    }

    iterator find(const T& data) {
        list_t<T>::iterator it = begin();
        while (it != end() && *it != data) {
            it++;
        }

        return it;
    }

    iterator begin() {
        return list_t::iterator(m_head);
    }
    iterator end() {
        return list_t::iterator(NULL);
    }

    spinlock_t* get_lock() {
        return &m_lock;
    }

    uint32 size() {
        return m_size;
    }

private:
    list_node_t<T>*     m_head;
    list_node_t<T>*     m_tail;
    uint32              m_size;
    object_pool_t*      m_pool;
    spinlock_t          m_lock;
    object_pool_t       m_big_pool;
};

#endif

