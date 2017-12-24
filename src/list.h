/*
 * guzhoudiaoke@126.com
 * 2017-12-24
 */

#ifndef _LIST_H_
#define _LIST_H_

#include "types.h"
#include "spinlock.h"
#include "babyos.h"

/*
 * NOTE: this list class 'DO NOT' alloc any memory,
 * just make m_data point to the added data
 */ 
class list_node_t {
public:
    void init(void* data) {
        m_data = data;
        m_prev = NULL;
        m_next = NULL;
    }

public:
    void       * m_data;
	list_node_t* m_next;
    list_node_t* m_prev;
};

template<typename T> class list_t {
public:
    class iterator {
    public:
        iterator() {
            m_ptr = NULL;
        }
        iterator(list_node_t* ptr) {
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
        T operator * () {
            return * ((T *)m_ptr->m_data);
        }

        friend class list_t;
    private:
        list_node_t* m_ptr;
    };

    void init() {
        m_head = NULL;
        m_tail = NULL;
        m_size = 0;
        m_lock.init();
    }

    list_node_t* alloc_node(T* data) {
        list_node_t* node = (list_node_t *) os()->get_obj_pool(LIST_POOL)->alloc_from_pool();
        node->init(data);
        return node;
    }

    void free_node(list_node_t* node) {
        os()->get_obj_pool(LIST_POOL)->free_object((void *) node);
    }

    bool empty() {
        return m_size == 0;
    }

    bool push_front_nolock(T* data) {
        list_node_t* node = alloc_node(data);
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
    bool push_front(T* data) {
        locker_t locker(m_lock);
        return push_front_nolock(data);
    }

    bool push_back_nolock(T* data) {
        list_node_t* node = alloc_node(data);
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
    bool push_back(T* data) {
        locker_t locker(m_lock);
        return push_back_nolock(data);
    }

    list_t<T>::iterator insert(list_t<T>::iterator &it, T* data) {
        locker_t locker(m_lock);

        // insert before end, just push back
        if (it.m_ptr == NULL) {
            push_back_nolock(data);
            return list_t::iterator(m_tail);
        }

        // insert before front
        if (it.m_ptr->m_prev == NULL) {
            push_front_nolock(data);
            return list_t::iterator(m_head);
        }

        // insert middle
        list_node_t* node = alloc_node(data);
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
        locker_t locker(m_lock);
        list_t<T>::iterator ret = it;

        // error erase
        if (m_size == 0) {
            return ret;
        }

        m_size--;

        // no node left
        if (m_size == 0) {
            m_head = m_tail = NULL;
            free_node(it.m_ptr);
            return list_t::iterator(NULL);
        }

        // erase head
        if (m_head == it.m_ptr) {
            m_head = m_head->m_next;
            free_node(it.m_ptr);
            return list_t::iterator(m_head);
        }

        // erase tail
        if (m_tail == it.m_ptr) {
            m_tail = m_tail->m_prev;
            m_tail->m_next = NULL;
            free_node(it.m_ptr);
            return list_t::iterator(m_tail);
        }

        // erase middle node
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

    iterator begin() {
        return list_t::iterator(m_head);
    }
    iterator end() {
        return list_t::iterator(NULL);
    }

private:
    list_node_t* m_head;
    list_node_t* m_tail;
    uint32       m_size;
    spinlock_t   m_lock;
};

#endif

