#pragma once


template <typename T, T TNul = T()>
class UniqueHandle
{
public:
  UniqueHandle(std::nullptr_t = nullptr)
    :m_id(TNul)
  { }
  UniqueHandle(T x)
    :m_id(x)
  { }
  explicit operator bool() const { return m_id != TNul; }

  operator T&() { return m_id; }
  operator T() const { return m_id; }

  T *operator&() { return &m_id; }
  const T *operator&() const { return &m_id; }

  friend bool operator == (UniqueHandle a, UniqueHandle b) { return a.m_id == b.m_id; }
  friend bool operator != (UniqueHandle a, UniqueHandle b) { return a.m_id != b.m_id; }
  friend bool operator == (UniqueHandle a, std::nullptr_t) { return a.m_id == TNul; }
  friend bool operator != (UniqueHandle a, std::nullptr_t) { return a.m_id != TNul; }
  friend bool operator == (std::nullptr_t, UniqueHandle b) { return TNul == b.m_id; }
  friend bool operator != (std::nullptr_t, UniqueHandle b) { return TNul != b.m_id; }

private:
  T m_id;
};
