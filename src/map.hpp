/**
* implement a container like std::map
*/
#ifndef SJTU_MAP_HPP
#define SJTU_MAP_HPP

// only for std::less<T>
#include <functional>
#include <cstddef>
#include "utility.hpp"
#include "exceptions.hpp"

namespace sjtu {

template<
   class Key,
   class T,
   class Compare = std::less <Key>
   > class map {
  public:
   /**
  * the internal type of data.
  * it should have a default constructor, a copy constructor.
  * You can use sjtu::map as value_type by typedef.
    */
   typedef pair<const Key, T> value_type;

  private:
   enum Color { RED, BLACK };

   struct Node {
       value_type data;
       Node *left, *right, *parent;
       Color color;

       Node(const value_type &val, Color c = RED, Node *p = nullptr,
            Node *l = nullptr, Node *r = nullptr)
           : data(val), color(c), parent(p), left(l), right(r) {}
   };


   Node *root;
   Node *nil; // sentinel node for leaf nodes
   size_t map_size;
   Compare comp;

   // Helper functions for Red-Black Tree operations
   void leftRotate(Node *x) {
       Node *y = x->right;
       x->right = y->left;
       if (y->left != nil) y->left->parent = x;
       y->parent = x->parent;
       if (x->parent == nil) root = y;
       else if (x == x->parent->left) x->parent->left = y;
       else x->parent->right = y;
       y->left = x;
       x->parent = y;
   }

   void rightRotate(Node *y) {
       Node *x = y->left;
       y->left = x->right;
       if (x->right != nil) x->right->parent = y;
       x->parent = y->parent;
       if (y->parent == nil) root = x;
       else if (y == y->parent->left) y->parent->left = x;
       else y->parent->right = x;
       x->right = y;
       y->parent = x;
   }

   void insertFixup(Node *z) {
       while (z->parent->color == RED) {
           if (z->parent == z->parent->parent->left) {
               Node *y = z->parent->parent->right;
               if (y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->right) {
                       z = z->parent;
                       leftRotate(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   rightRotate(z->parent->parent);
               }
           } else {
               Node *y = z->parent->parent->left;
               if (y->color == RED) {
                   z->parent->color = BLACK;
                   y->color = BLACK;
                   z->parent->parent->color = RED;
                   z = z->parent->parent;
               } else {
                   if (z == z->parent->left) {
                       z = z->parent;
                       rightRotate(z);
                   }
                   z->parent->color = BLACK;
                   z->parent->parent->color = RED;
                   leftRotate(z->parent->parent);
               }
           }
       }
       root->color = BLACK;
   }

   void transplant(Node *u, Node *v) {
       if (u->parent == nil) root = v;
       else if (u == u->parent->left) u->parent->left = v;
       else u->parent->right = v;
       v->parent = u->parent;
   }

   Node *minimum(Node *x) const {
       while (x->left != nil) x = x->left;
       return x;
   }

   void eraseFixup(Node *x) {
       while (x != root && x->color == BLACK) {
           if (x == x->parent->left) {
               Node *w = x->parent->right;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   leftRotate(x->parent);
                   w = x->parent->right;
               }
               if (w->left->color == BLACK && w->right->color == BLACK) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->right->color == BLACK) {
                       w->left->color = BLACK;
                       w->color = RED;
                       rightRotate(w);
                       w = x->parent->right;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   w->right->color = BLACK;
                   leftRotate(x->parent);
                   x = root;
               }
           } else {
               Node *w = x->parent->left;
               if (w->color == RED) {
                   w->color = BLACK;
                   x->parent->color = RED;
                   rightRotate(x->parent);
                   w = x->parent->left;
               }
               if (w->right->color == BLACK && w->left->color == BLACK) {
                   w->color = RED;
                   x = x->parent;
               } else {
                   if (w->left->color == BLACK) {
                       w->right->color = BLACK;
                       w->color = RED;
                       leftRotate(w);
                       w = x->parent->left;
                   }
                   w->color = x->parent->color;
                   x->parent->color = BLACK;
                   w->left->color = BLACK;
                   rightRotate(x->parent);
                   x = root;
               }
           }
       }
       x->color = BLACK;
   }

   void clearHelper(Node *node) {
       if (node == nil) return;
       clearHelper(node->left);
       clearHelper(node->right);
       delete node;
   }

   Node *copyHelper(Node *node, Node *parent) {
       if (node == nil) return nil;
       Node *newNode = new Node(node->data, node->color, parent);
       newNode->left = copyHelper(node->left, newNode);
       newNode->right = copyHelper(node->right, newNode);
       return newNode;
   }

   Node *findHelper(const Key &key) const {
       Node *current = root;
       while (current != nil) {
           if (comp(key, current->data.first)) current = current->left;
           else if (comp(current->data.first, key)) current = current->right;
           else return current;
       }
       return nil;
   }

  public:
   /**
  * see BidirectionalIterator at CppReference for help.
  *
  * if there is anything wrong throw invalid_iterator.
  *     like it = map.begin(); --it;
  *       or it = map.end(); ++end();
    */
   class const_iterator;
   class iterator {
      private:
       Node *current;
       const map *container;

      public:
       iterator(Node *node = nullptr, const map *cont = nullptr) : current(node), container(cont) {}

       iterator(const iterator &other) : current(other.current), container(other.container) {}

       /**
    * TODO iter++
        */
       iterator operator++(int) {
           iterator temp = *this;
           ++(*this);
           return temp;
       }

       /**
    * TODO ++iter
        */
       iterator &operator++() {
           if (current == container->nil) throw invalid_iterator();
           if (current->right != container->nil) {
               current = container->minimum(current->right);
           } else {
               Node *parent = current->parent;
               while (parent != container->nil && current == parent->right) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }
           return *this;
       }

       /**
    * TODO iter--
        */
       iterator operator--(int) {
           iterator temp = *this;
           --(*this);
           return temp;
       }

       /**
    * TODO --iter
        */
       iterator &operator--() {
           if (current == container->nil) {
               // end() iterator, go to last element
               if (container->root == container->nil) throw invalid_iterator();
               current = container->root;
               while (current->right != container->nil) current = current->right;
           } else {
               if (current->left != container->nil) {
                   current = current->left;
                   while (current->right != container->nil) current = current->right;
               } else {
                   Node *parent = current->parent;
                   while (parent != container->nil && current == parent->left) {
                       current = parent;
                       parent = parent->parent;
                   }
                   current = parent;
                   if (current == container->nil) throw invalid_iterator();
               }
           }
           return *this;
       }

       /**
    * a operator to check whether two iterators are same (pointing to the same memory).
        */
       value_type &operator*() const {
           if (current == container->nil) throw invalid_iterator();
           return current->data;
       }

       bool operator==(const iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator==(const const_iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       /**
    * some other operator for iterator.
        */
       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       /**
    * for the support of it->first.
    * See <http://kelvinh.github.io/blog/2013/11/20/overloading-of-member-access-operator-dash-greater-than-symbol-in-cpp/> for help.
        */
       value_type *operator->() const noexcept {
           if (current == container->nil) throw invalid_iterator();
           return &(current->data);
       }

       friend class map;
       friend class const_iterator;
   };

   class const_iterator {
      private:
       const Node *current;
       const map *container;

      public:
       const_iterator(const Node *node = nullptr, const map *cont = nullptr) : current(node), container(cont) {}

       const_iterator(const const_iterator &other) : current(other.current), container(other.container) {}

       const_iterator(const iterator &other) : current(other.current), container(other.container) {}

       const_iterator &operator=(const const_iterator &other) {
           current = other.current;
           container = other.container;
           return *this;
       }

       const_iterator operator++(int) {
           const_iterator temp = *this;
           ++(*this);
           return temp;
       }

       const_iterator &operator++() {
           if (current == container->nil) throw invalid_iterator();
           if (current->right != container->nil) {
               current = container->minimum(current->right);
           } else {
               const Node *parent = current->parent;
               while (parent != container->nil && current == parent->right) {
                   current = parent;
                   parent = parent->parent;
               }
               current = parent;
           }
           return *this;
       }

       const_iterator operator--(int) {
           const_iterator temp = *this;
           --(*this);
           return temp;
       }

       const_iterator &operator--() {
           if (current == container->nil) {
               if (container->root == container->nil) throw invalid_iterator();
               current = container->root;
               while (current->right != container->nil) current = current->right;
           } else {
               if (current->left != container->nil) {
                   current = current->left;
                   while (current->right != container->nil) current = current->right;
               } else {
                   const Node *parent = current->parent;
                   while (parent != container->nil && current == parent->left) {
                       current = parent;
                       parent = parent->parent;
                   }
                   current = parent;
                   if (current == container->nil) throw invalid_iterator();
               }
           }
           return *this;
       }

       const value_type &operator*() const {
           if (current == container->nil) throw invalid_iterator();
           return current->data;
       }

       bool operator==(const const_iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator==(const iterator &rhs) const {
           return current == rhs.current && container == rhs.container;
       }

       bool operator!=(const const_iterator &rhs) const {
           return !(*this == rhs);
       }

       bool operator!=(const iterator &rhs) const {
           return !(*this == rhs);
       }

       const value_type *operator->() const noexcept {
           if (current == container->nil) throw invalid_iterator();
           return &(current->data);
       }

       friend class map;
   };

   /**
  * TODO two constructors
    */
   map() : map_size(0), comp() {
       // Create nil node without storing actual data
       nil = new Node(value_type(Key(), T()), BLACK);
       nil->left = nil->right = nil->parent = nil;
       root = nil;
   }

   map(const map &other) : map_size(other.map_size), comp() {
       // Create nil node without storing actual data
       nil = new Node(value_type(Key(), T()), BLACK);
       nil->left = nil->right = nil->parent = nil;
       root = copyHelper(other.root, nil);
   }

   /**
  * TODO assignment operator
    */
   map &operator=(const map &other) {
       if (this != &other) {
           clear();
           delete nil;
           // Create nil node without storing actual data
           nil = new Node(value_type(Key(), T()), BLACK);
           nil->left = nil->right = nil->parent = nil;
           root = copyHelper(other.root, nil);
           map_size = other.map_size;
       }
       return *this;
   }

   /**
  * TODO Destructors
    */
   ~map() {
       clear();
       delete nil;
   }

   /**
  * TODO
  * access specified element with bounds checking
  * Returns a reference to the mapped value of the element with key equivalent to key.
  * If no such element exists, an exception of type `index_out_of_bound'
    */
   T &at(const Key &key) {
       Node *node = findHelper(key);
       if (node == nil) throw index_out_of_bound();
       return node->data.second;
   }

   const T &at(const Key &key) const {
       const Node *node = findHelper(key);
       if (node == nil) throw index_out_of_bound();
       return node->data.second;
   }

   /**
  * TODO
  * access specified element
  * Returns a reference to the value that is mapped to a key equivalent to key,
  *   performing an insertion if such key does not already exist.
    */
   T &operator[](const Key &key) {
       Node *node = findHelper(key);
       if (node != nil) return node->data.second;

       // Insert new element with default constructed T
       T default_value;
       auto result = insert(value_type(key, default_value));
       return result.first->second;
   }

   /**
  * behave like at() throw index_out_of_bound if such key does not exist.
    */
   const T &operator[](const Key &key) const {
       return at(key);
   }

   /**
  * return a iterator to the beginning
    */
   iterator begin() {
       if (root == nil) return iterator(nil, this);
       return iterator(minimum(root), this);
   }

   const_iterator cbegin() const {
       if (root == nil) return const_iterator(nil, this);
       return const_iterator(minimum(root), this);
   }

   /**
  * return a iterator to the end
  * in fact, it returns past-the-end.
    */
   iterator end() {
       return iterator(nil, this);
   }

   const_iterator cend() const {
       return const_iterator(nil, this);
   }

   /**
  * checks whether the container is empty
  * return true if empty, otherwise false.
    */
   bool empty() const {
       return map_size == 0;
   }

   /**
  * returns the number of elements.
    */
   size_t size() const {
       return map_size;
   }

   /**
  * clears the contents
    */
   void clear() {
       clearHelper(root);
       root = nil;
       map_size = 0;
   }

   /**
  * insert an element.
  * return a pair, the first of the pair is
  *   the iterator to the new element (or the element that prevented the insertion),
  *   the second one is true if insert successfully, or false.
    */
   pair<iterator, bool> insert(const value_type &value) {
       Node *y = nil;
       Node *x = root;

       while (x != nil) {
           y = x;
           if (comp(value.first, x->data.first)) x = x->left;
           else if (comp(x->data.first, value.first)) x = x->right;
           else return pair<iterator, bool>(iterator(x, this), false);
       }

       Node *z = new Node(value, RED, y);
       z->left = z->right = nil;

       if (y == nil) root = z;
       else if (comp(value.first, y->data.first)) y->left = z;
       else y->right = z;

       insertFixup(z);
       map_size++;
       return pair<iterator, bool>(iterator(z, this), true);
   }

   /**
  * erase the element at pos.
  *
  * throw if pos pointed to a bad element (pos == this->end() || pos points an element out of this)
    */
   void erase(iterator pos) {
       if (pos.container != this || pos.current == nil) throw invalid_iterator();

       Node *z = pos.current;
       Node *y = z;
       Node *x;
       Color y_original_color = y->color;

       if (z->left == nil) {
           x = z->right;
           transplant(z, z->right);
       } else if (z->right == nil) {
           x = z->left;
           transplant(z, z->left);
       } else {
           y = minimum(z->right);
           y_original_color = y->color;
           x = y->right;
           if (y->parent == z) {
               x->parent = y;
           } else {
               transplant(y, y->right);
               y->right = z->right;
               y->right->parent = y;
           }
           transplant(z, y);
           y->left = z->left;
           y->left->parent = y;
           y->color = z->color;
       }

       if (y_original_color == BLACK) eraseFixup(x);
       delete z;
       map_size--;
   }

   /**
  * Returns the number of elements with key
  *   that compares equivalent to the specified argument,
  *   which is either 1 or 0
  *     since this container does not allow duplicates.
  * The default method of check the equivalence is !(a < b || b > a)
    */
   size_t count(const Key &key) const {
       return findHelper(key) != nil ? 1 : 0;
   }

   /**
  * Finds an element with key equivalent to key.
  * key value of the element to search for.
  * Iterator to an element with key equivalent to key.
  *   If no such element is found, past-the-end (see end()) iterator is returned.
    */
   iterator find(const Key &key) {
       Node *node = findHelper(key);
       return iterator(node, this);
   }

   const_iterator find(const Key &key) const {
       const Node *node = findHelper(key);
       return const_iterator(node, this);
   }
};

}

#endif