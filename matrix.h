#ifndef MATRIX_H
#define MATRIX_H

#include <unordered_map>
#include <memory>

namespace matrix
{
template<class T, size_t D, T DefaultValue>
class Matrix;

namespace internal
{

template<class T, size_t D>
struct Node
{
    Node() = default;
    explicit Node(size_t ind, Node<T,D+1>* parent = nullptr):ind(ind), parent(parent){}

    Node(const Node& rhs, Node<T,D+1>* parent = nullptr):
        ind(ind), parent(parent)
    {
        for(const auto& child : rhs.children)
        {
            auto node = std::make_shared<Node<T, D-1>>(&child.second, this);
            children.emplace(child.first, std::move(node) );
        }
    }

    size_t size() const
    {
        size_t result = 0;
        for(const auto& child : children)
            result += child.second->size();
        return result;
    }

    size_t ind = 0;
    std::unordered_map<size_t, std::shared_ptr<Node<T, D-1>> > children;
    Node<T,D+1>* parent = nullptr;
};

template<class T>
struct Node<T, 0>
{
    Node() = default;
    explicit Node(Node<T,1>* parent = nullptr): parent(parent) {}

    Node(size_t ind, Node<T,1>* parent = nullptr): parent(parent), ind(ind){}

    Node(size_t ind, const Node& rhs, Node<T, 1>* parent = nullptr):
        ind(ind), value(rhs.value), parent(parent){}

    Node(const T& val):value(val){}

    size_t size() const {return 1;}

    size_t ind = 0;
    T value;
    Node<T,1>* parent = nullptr;
};

template<class T, size_t D>
using NodeSh = std::shared_ptr<internal::Node<T,D>>;

//template<class T, size_t D>
//struct MatrixIterator
//{
//    typedef std::bidirectional_iterator_tag iterator_category;

//    MatrixIterator() = default;

//    explicit MatrixIterator(Node<* node) noexcept
//        : node(node) { }

//    T& operator*() const noexcept
//    {
//        return static_cast<Node<T>*>(node)->value;
//    }

//    T* operator->() const noexcept
//    {
//        return &(static_cast<Node<T>*>(node)->value);
//    }

//    SListIterator& operator++() noexcept
//    {
//        node = node->next;
//        return *this;
//    }

//    SListIterator operator++(int) noexcept
//    {
//        SListIterator tmp = *this;
//        node = node->next;
//        return tmp;
//    }

//    bool operator==(const SListIterator& rhs) const noexcept
//    { return node == rhs.node; }

//    bool operator!=(const SListIterator& rhs) const noexcept
//    { return node != rhs.node; }

//    BaseNode* node;
//};

}

template<class T, size_t D, T DefaultValue = T()>
class Matrix
{    
public:
    Matrix()
    {
        m_node = std::make_shared<internal::Node<T,D>>();
    }

    Matrix(const Matrix& rhs)
    {
        for(const auto& node : rhs.m_node->children)
            m_node->children.emplace(node.first,
                            std::make_shared<internal::Node<T,D-1>>(*node.second) );
    }

    Matrix<T,D-1, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_node->children.find(ind);
        if(itr != m_node->children.end() )
           return Matrix<T,D-1,DefaultValue>(itr->second);
        else
        {
            m_node->children.emplace(ind, std::make_shared<internal::Node<T,D-1>>(ind, m_node.get() ) );
            return Matrix<T, D-1, DefaultValue>(m_node->children[ind]);
        }
    }

    size_t size() const
    {
        return m_node->size();
    }

    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(internal::NodeSh<T,D> node): m_node(std::move(node) ){}

    internal::NodeSh<T,D> m_node;
};

template<class T, T DefaultValue>
class Matrix<T,1,DefaultValue>
{
public:
    Matrix()
    {
        m_node = std::make_shared<internal::Node<T,1>>();
    }

    Matrix(const Matrix& rhs)
    {
        for(const auto& node : rhs.m_node->children)
            m_node->children.emplace(node.first,
                            std::make_shared<internal::Node<T,0>>(*node.second) );
    }

    Matrix<T,0, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_node->children.find(ind);
        if(itr != m_node->children.end() )
           return Matrix<T,0,DefaultValue>(itr->second);
        else
        {
            return Matrix<T,0,DefaultValue>(m_node.get(), ind);
        }
    }

    size_t size() const
    {
        return m_node->size();
    }

    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(internal::NodeSh<T,1> node): m_node(std::move(node) ){}

    internal::NodeSh<T,1> m_node;
};

template<class T, T DefaultValue>
class Matrix<T, 0, DefaultValue>
{
    using NodeSh = std::shared_ptr<internal::Node<T,0>>;
public:
    Matrix() = default;

    Matrix(const T& v)
    {
        m_node = std::make_shared<internal::Node<T,0>>(v);
    }

    template<class U, size_t D, T Default>
    Matrix(const Matrix<U,D,Default>& rhs)
    {
        if(rhs.m_node)
            m_node = std::make_shared<internal::Node<T,0>>(rhs.m_node->value);
        else
            m_node = std::make_shared<internal::Node<T,0>>(Default);
    }

    Matrix(const Matrix& rhs)
    {
        if(rhs.m_node)
            m_node = std::make_shared<internal::Node<T,0>>(rhs.m_node->value);
    }

    ~Matrix()
    {
        if(m_node)
        {
            auto parent = m_node->parent;
            if(parent && (m_node->value == DefaultValue) )
            {
                auto itr = parent->children.find(m_node->ind);
                if(itr != parent->children.end() )
                    parent->children.erase(itr);

            }
        }
    }

    size_t size() const {return 1;}

    operator T() const {return m_node ? m_node->value : DefaultValue;}

    template<class U>
    Matrix& operator=(const U& v)
    {
        if(!m_node)
        {
            m_node = std::make_shared<internal::Node<T,0>>(v);
            if(m_parent)
            {
                m_node->parent = m_parent;
                m_node->ind = m_ind;
                m_node->parent->children.emplace(m_ind, m_node);
            }
        }
        else
            m_node->value = v;
        return *this;
    }

    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(internal::Node<T,1>* parent, size_t ind):m_ind(ind), m_parent(parent){}

    Matrix(NodeSh node):
        m_parent(node->parent), m_node(std::move(node) )
    {}

    size_t m_ind = 0;
    internal::Node<T,1>* m_parent = nullptr;
    NodeSh m_node = nullptr;
};

}

#endif
