#ifndef MATRIX_H
#define MATRIX_H

#include <cstdlib>
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
    Node(size_t ind, Node<T,D+1>* parent = nullptr):ind(ind),parent(parent){}
    size_t ind;
    std::unordered_map<size_t, std::unique_ptr<Node<T, D-1>> > children;
    Node<T,D+1>* parent = nullptr;
};

template<class T>
struct Node<T, 0>
{
    Node() = default;
    Node(size_t ind, Node<T,1>* parent = nullptr):ind(ind),parent(parent){}
    Node(const T& val):value(val){}
    size_t ind = 0;
    T value;
    Node<T,1>* parent = nullptr;
};

template<class T, size_t D>
using NodeSh = std::shared_ptr<internal::Node<T,D>>;

}

template<class T, size_t D, T DefaultValue = T()>
class Matrix
{    
public:
    Matrix() = default;
    Matrix<T,D-1, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_nodes.find(ind);
        if(itr != m_nodes.end() )
           return Matrix<T,D-1,DefaultValue>(itr->second);
        else
        {
            m_nodes.emplace(ind, std::make_shared<internal::Node<T,D-1>>() );
            return Matrix<T, D-1, DefaultValue>(m_nodes[ind]);
        }
    }
    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(const internal::NodeSh<T,D>& node)
    {
        for(const auto& child : node->children)
            m_nodes.emplace(child->ind, child);
    }
    std::unordered_map<size_t, internal::NodeSh<T,D-1>> m_nodes;
};

template<class T, T DefaultValue>
class Matrix<T, 1, DefaultValue>
{
public:
    Matrix() = default;
    Matrix<T, 0, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_nodes.find(ind);
        if(itr != m_nodes.end() )
           return Matrix<T, 0, DefaultValue>(itr->second, this);
        else
        {
            m_nodes.emplace(ind, std::make_shared<internal::Node<T, 0>>() );
            return Matrix<T, 0, DefaultValue>(m_nodes[ind], this);
        }
    }
    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(const internal::NodeSh<T, 1>& node)
    {
        for(const auto& child : node->children)
            m_nodes.emplace(child->ind, child);
    }
    std::unordered_map<size_t, internal::NodeSh<T, 0>> m_nodes;
};

template<class T, T DefaultValue>
class Matrix<T, 0, DefaultValue>
{
    using NodeSh = std::shared_ptr<internal::Node<T,0>>;
public:
    Matrix() {}

    Matrix(const T& v)
    {
        m_node = std::make_shared<internal::Node<T,0>>(v);
    }

    template<class U, size_t D, T Defa>
    Matrix(const Matrix<U,D,Defa>& rhs)
    {
        if(rhs.m_node)
            m_node = std::make_shared<internal::Node<T,0>>(rhs.m_node->value);
    }

    Matrix(const Matrix& rhs)
    {
        if(rhs.m_node)
            m_node = std::make_shared<internal::Node<T,0>>(rhs.m_node->value);
    }

    Matrix(Matrix&& ppp)
    {
        int a = 1;
    }

    ~Matrix()
    {
        if(parent && (!m_node || m_node->value == DefaultValue) )
        {
            auto itr = parent->m_nodes.find(m_node->ind);
            if(itr != parent->m_nodes.end() )
                parent->m_nodes.erase(itr);
        }
    }
    operator T(){return m_node ? m_node->value : DefaultValue;}
    operator T() const {return m_node ? m_node->value : DefaultValue;}
    Matrix& operator=(const T& v)
    {
        if(!m_node)
            m_node = std::make_shared<internal::Node<T,0>>(v);
        m_node->value = v;
        return *this;
    }
    template<class T1, std::size_t D1, T1 DefaultValue1> friend class Matrix;
private:
    Matrix(NodeSh node, Matrix<T, 1, DefaultValue>* parent):
        m_node(std::move(node) ), parent(parent)
    {}
    NodeSh m_node = nullptr;
    Matrix<T, 1, DefaultValue>* parent = nullptr;
};

}

#endif
