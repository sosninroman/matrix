#ifndef MATRIX_H
#define MATRIX_H

#include <map>
#include <memory>

namespace matrix
{
template<class T, size_t D, T DefaultValue>
class Matrix;

namespace internal
{

template<class T, size_t D>
struct Node;

template<class T>
struct BaseNode
{
    BaseNode() = default;
    BaseNode(size_t ind, BaseNode* p):ind(ind), parent(p){}

    virtual ~BaseNode() = default;

    BaseNode* next()
    {
        if(parent)
        {
            std::map<size_t, std::shared_ptr<BaseNode>>& brothers = parent->children;
            auto lb = brothers.upper_bound(ind);
            if(lb != brothers.end() )
                return lb->second.get();
            else
            {
                BaseNode* nextPa = parent->next();
                if(nextPa)
                {
                    while(nextPa && nextPa->children.empty() )
                        nextPa = nextPa->next();
                    if(nextPa && !nextPa->children.empty() )
                    {
                        return nextPa->children.begin()->second.get();
                    }
                }
            }
        }
        return nullptr;
    }

    virtual size_t size() const = 0;
    virtual Node<T,0>* begin() = 0;

    size_t ind;
    BaseNode* parent = nullptr;
    std::map<size_t, std::shared_ptr<BaseNode>> children;
};

template<class T, size_t D>
struct Node : public BaseNode<T>
{
    Node() = default;
    explicit Node(size_t ind, Node<T,D+1>* parent = nullptr): BaseNode<T>(ind, parent){}

    Node(const Node& rhs, Node<T,D+1>* parent = nullptr):
        BaseNode<T>::ind(rhs.ind), BaseNode<T>::parent(parent)
    {
        for(const auto& child : rhs.children)
        {
            auto node = std::make_shared<Node<T, D-1>>(&child.second, this);
            BaseNode<T>::children.emplace(child.first, std::move(node) );
        }
    }

    size_t size() const override
    {
        size_t result = 0;
        for(const auto& child : BaseNode<T>::children)
            result += child.second->size();
        return result;
    }

    Node<T,0>* begin() override
    {
        if(!BaseNode<T>::children.empty() )
        {
            return BaseNode<T>::children.begin()->second->begin();
        }
        return nullptr;
    }

    Node<T,D+1>* parent = nullptr;
};

template<class T>
struct Node<T, 0> : public BaseNode<T>
{
    Node() = default;
    explicit Node(Node<T,1>* parent = nullptr): BaseNode<T>::parent(parent) {}

    Node(size_t ind, Node<T,1>* parent = nullptr): BaseNode<T>::parent(parent), BaseNode<T>::ind(ind){}

    Node(size_t ind, const Node& rhs, Node<T, 1>* parent = nullptr):
        BaseNode<T>::ind(ind), value(rhs.value), BaseNode<T>::parent(parent){}

    Node(const T& val):value(val){}

    Node<T,0>* begin() override
    {
        return this;
    }

    size_t size() const override {return 1;}

    T value;
};

template<class T, size_t D>
using NodeSh = std::shared_ptr<internal::Node<T,D>>;

template<class T, size_t D>
struct MatrixIterator
{
    typedef std::bidirectional_iterator_tag iterator_category;

    MatrixIterator() = default;

    explicit MatrixIterator(Node<T,0>* node) noexcept
        : node(node) { }

    T& operator*() const noexcept
    {
        return node->value;
    }

    T* operator->() const noexcept
    {
        return &(node->value);
    }

    MatrixIterator& operator++() noexcept
    {
        node = static_cast<Node<T,0>*>(node->next() );
        return *this;
    }

    MatrixIterator operator++(int) noexcept
    {
        MatrixIterator tmp = *this;
        node = node->next();
        return tmp;
    }

    bool operator==(const MatrixIterator& rhs) const noexcept
    { return node == rhs.node; }

    bool operator!=(const MatrixIterator& rhs) const noexcept
    { return node != rhs.node; }

    Node<T,0>* node;
};

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
           return Matrix<T,D-1,DefaultValue>(std::dynamic_pointer_cast<internal::Node<T,D-1>>(itr->second) );
        else
        {
            auto ptr = std::make_shared<internal::Node<T,D-1>>(ind, m_node.get() );
            m_node->children.emplace(ind,ptr);
            return Matrix<T, D-1, DefaultValue>(ptr);
        }
    }

    internal::MatrixIterator<T,D> begin()
    {
        if(m_node)
            return internal::MatrixIterator<T,D>(m_node->begin() );
        else
            return internal::MatrixIterator<T,D>();
    }

    internal::MatrixIterator<T,D> end()
    {
        return internal::MatrixIterator<T,D>();
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
                std::make_shared<internal::Node<T,0>>(*std::dynamic_pointer_cast<internal::Node<T,0>>(node.second) ) );
    }

    internal::MatrixIterator<T,1> begin()
    {
        if(m_node)
            return internal::MatrixIterator<T,1>(m_node->begin() );
        else
            return internal::MatrixIterator<T,1>();
    }

    internal::MatrixIterator<T,1> end()
    {
        return internal::MatrixIterator<T,1>();
    }

    Matrix<T,0, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_node->children.find(ind);
        if(itr != m_node->children.end() )
           return Matrix<T,0,DefaultValue>( std::dynamic_pointer_cast<internal::Node<T,0>>(itr->second) );
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

    internal::MatrixIterator<T,0> begin()
    {
        if(m_node)
        {
            internal::Node<T,0>* n = m_node->begin();
            return internal::MatrixIterator<T,0>(n);
        }
        else
            return internal::MatrixIterator<T,0>();
    }

    internal::MatrixIterator<T,0> end()
    {
        return internal::MatrixIterator<T,0>();
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
    internal::BaseNode<T>* m_parent = nullptr;
    NodeSh m_node = nullptr;
};

}

#endif
