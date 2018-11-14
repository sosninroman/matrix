#ifndef MATRIX_H
#define MATRIX_H

#include <map>
#include <memory>
#include <cassert>
#include <tuple>

namespace matrix
{
template<class T, size_t D, T DefaultValue>
class Matrix;

namespace internal
{

template <class T, std::size_t N, std::size_t... Is>
auto unpack_impl(std::array<T, N> &arr, std::index_sequence<Is...>)
{
    return std::tie(arr[Is]... );
}

template <class T, std::size_t N>
auto unpack(std::array<T, N> &arr)
{
    return unpack_impl(arr, std::make_index_sequence<N>{});
}

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
    virtual Node<T,0>* firstChild() = 0;

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
        BaseNode<T>(rhs.ind, parent)
    {
        for(const auto& child : rhs.children)
        {
            auto node = std::make_shared<Node<T, D-1>>(*child.second, this);
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

    Node<T,0>* firstChild() override
    {
        if(!BaseNode<T>::children.empty() )
        {
            return BaseNode<T>::children.begin()->second->firstChild();
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

    explicit Node(size_t ind, Node<T,1>* parent = nullptr): BaseNode<T>(ind, parent){}

    Node(size_t ind, const Node& rhs, Node<T, 1>* parent = nullptr):
        BaseNode<T>::ind(ind), value(rhs.value), BaseNode<T>::parent(parent){}

    Node(const T& val):value(val){}

    Node<T,0>* firstChild() override
    {
        return this;
    }

    size_t size() const override {return 1;}

    T value;
};

template<class T, size_t D>
using NodeSh = std::shared_ptr<internal::Node<T,D>>;

template<class T, size_t D>
class MatrixIterator
{
public:
    typedef std::bidirectional_iterator_tag iterator_category;

    MatrixIterator() = default;

    explicit MatrixIterator(Node<T,0>* node) noexcept
        : node(node)
    {
        recalcPosition();
    }

    void recalcPosition()
    {
        if(node)
        {
            BaseNode<T>* ptr = node;
            for(int i = D-1; i >= 0; --i)
            {
                assert(ptr);
                position[i] = ptr->ind;
                ptr = ptr->parent;
            }
        }
        else
        {
            for(int i = 0; i < D; ++i)
            {
                position[i] = 0;
            }
        }
    }

    auto operator*() const noexcept
    {
        return std::tuple_cat(unpack(position), std::tie(node->value));
    }

    T* operator->() const noexcept
    {
        return &(node->value);
    }

    T& value() const noexcept
    {
        return node->value;
    }

    MatrixIterator& operator++() noexcept
    {
        node = static_cast<Node<T,0>*>(node->next() );
        recalcPosition();
        return *this;
    }

    MatrixIterator operator++(int) noexcept
    {
        MatrixIterator tmp = *this;
        node = node->next();
        recalcPosition();
        return tmp;
    }

    template<class... Args>
    operator std::tuple<Args...>()
    {

        return std::tuple_cat(unpack(position), std::tie(node->value));
    }

    bool operator==(const MatrixIterator& rhs) const noexcept
    { return node == rhs.node; }

    bool operator!=(const MatrixIterator& rhs) const noexcept
    { return node != rhs.node; }

private:
    Node<T,0>* node;
    mutable std::array<int, D> position;
};

template<class T, size_t D>
class ConstMatrixIterator
{
public:
    typedef std::bidirectional_iterator_tag iterator_category;

    ConstMatrixIterator() = default;

    explicit ConstMatrixIterator(Node<T,0>* node) noexcept
        : node(node)
    {
        recalcPosition();
    }

    void recalcPosition()
    {
        if(node)
        {
            BaseNode<T>* ptr = node;
            for(int i = D-1; i >= 0; --i)
            {
                assert(ptr);
                position[i] = ptr->ind;
                ptr = ptr->parent;
            }
        }
        else
        {
            for(int i = 0; i < D; ++i)
            {
                position[i] = 0;
            }
        }
    }

    auto operator*() const noexcept
    {
        return std::tuple_cat(unpack(position), std::tie(node->value));
    }

    const T* operator->() const noexcept
    {
        return &(node->value);
    }

    const T& value() const noexcept
    {
        return node->value;
    }

    ConstMatrixIterator& operator++() noexcept
    {
        node = static_cast<Node<T,0>*>(node->next() );
        recalcPosition();
        return *this;
    }

    ConstMatrixIterator operator++(int) noexcept
    {
        ConstMatrixIterator tmp = *this;
        node = node->next();
        recalcPosition();
        return tmp;
    }

    template<class... Args>
    operator std::tuple<Args...>()
    {

        return std::tuple_cat(unpack(position), std::tie(node->value));
    }

    bool operator==(const ConstMatrixIterator& rhs) const noexcept
    { return node == rhs.node; }

    bool operator!=(const ConstMatrixIterator& rhs) const noexcept
    { return node != rhs.node; }

private:
    Node<T,0>* node;
    mutable std::array<int, D> position;
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

    Matrix(const Matrix& rhs):
        m_node(std::dynamic_pointer_cast<internal::Node<T,D>>(rhs.m_node))
    {
    }

    Matrix<T,D-1, DefaultValue> operator[](size_t ind)
    {
        auto itr = m_node->children.find(ind);
        if(itr != m_node->children.end() )
        {
           return Matrix<T,D-1,DefaultValue>(std::dynamic_pointer_cast<internal::Node<T,D-1>>(itr->second) );
        }
        else
        {
            auto ptr = std::make_shared<internal::Node<T,D-1>>(ind, m_node.get() );
            m_node->children.emplace(ind,ptr);
            return Matrix<T, D-1, DefaultValue>(ptr);
        }
    }

    const Matrix<T,D-1, DefaultValue> operator[](size_t ind) const
    {
        auto itr = m_node->children.find(ind);
        if(itr != m_node->children.end() )
        {
           return Matrix<T,D-1,DefaultValue>(std::dynamic_pointer_cast<internal::Node<T,D-1>>(itr->second) );
        }
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
            return internal::MatrixIterator<T,D>(m_node->firstChild() );
        else
            return internal::MatrixIterator<T,D>();
    }

    internal::MatrixIterator<T,D> end()
    {
        return internal::MatrixIterator<T,D>();
    }

    internal::ConstMatrixIterator<T,D> begin() const
    {
        if(m_node)
            return internal::ConstMatrixIterator<T,D>(m_node->firstChild() );
        else
            return internal::ConstMatrixIterator<T,D>();
    }

    internal::ConstMatrixIterator<T,D> end() const
    {
        return internal::ConstMatrixIterator<T,D>();
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

    Matrix(const Matrix& rhs):
        m_node(std::dynamic_pointer_cast<internal::Node<T,1>>(rhs.m_node))
    {
    }

    internal::MatrixIterator<T,1> begin()
    {
        if(m_node)
            return internal::MatrixIterator<T,1>(m_node->firstChild() );
        else
            return internal::MatrixIterator<T,1>();
    }

    internal::MatrixIterator<T,1> end()
    {
        return internal::MatrixIterator<T,1>();
    }

    internal::ConstMatrixIterator<T,1> begin() const
    {
        if(m_node)
            return internal::ConstMatrixIterator<T,1>(m_node->firstChild() );
        else
            return internal::ConstMatrixIterator<T,1>();
    }

    internal::ConstMatrixIterator<T,1> end() const
    {
        return internal::ConstMatrixIterator<T,1>();
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

    const Matrix<T,0, DefaultValue> operator[](size_t ind) const
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
            internal::Node<T,0>* n = m_node->firstChild();
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
