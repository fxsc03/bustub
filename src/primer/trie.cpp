#include "primer/trie.h"
#include <string_view>
#include "common/exception.h"

namespace bustub {

template <class T>
// done
// 内存泄漏了
auto Trie::Get(std::string_view key) const -> const T * {

// 
  // throw NotImplementedException("Trie::Get is not implemented.");

  if(root_ == nullptr) {
    return nullptr;
  }

  auto curr = root_;

  // // 防止key是空
  // if(key.empty()) {
  //   return nullptr;
  // }

  // 走key树，找到所需的位置
  for(size_t i = 0; i < key.size(); i ++ ) {
    // std::map<char, std::shared_ptr<const TrieNode>> children_;
    // 在map中,每个元素是一个std::pair<const Key, Value>
    // 定义一个迭代器来使用find找map中的值,children_是一个map容器,一层找一个key
    // trie是棵多叉树
    auto it = curr->children_.find(key[i]);
    if(it == curr->children_.end()) {
        // 如果没找到直接返回空指针
        return nullptr;
    }
    curr = it->second;
  }

  //在找到对应的节点后，使用dynamic_cast看看它是不是存有value
  auto node_with_value = dynamic_cast<const TrieNodeWithValue<T> *>(curr.get());
  if(node_with_value == nullptr) {
    return nullptr;
  }

  return node_with_value->value_.get();


  // You should walk through the trie to find the node corresponding to the key. If the node doesn't exist, return
  // nullptr. After you find the node, you should use `dynamic_cast` to cast it to `const TrieNodeWithValue<T> *`. If
  // dynamic_cast returns `nullptr`, it means the type of the value is mismatched, and you should return nullptr.
  // Otherwise, return the value.
  
}

template <class T>
auto Trie::Put(std::string_view key, T value) const -> Trie {
  // Note that `T` might be a non-copyable type. Always use `std::move` when creating `shared_ptr` on that value.
  // throw NotImplementedException("Trie::Put is not implemented.");
  // 参数K-V,先找到K节点，再传键值V，key是一串值，代表一条路径

  /*
  首先处理key是空的情况，再处理加节点的一般情况。
  */
  
  if(key.empty()) {
    std::shared_ptr<T> node_value = std::make_shared<T>(std::move(value));
    std::shared_ptr<TrieNodeWithValue<T>> new_root_ = nullptr;

    // 空key绑定的value就是Trie的根对应的值
    if(root_->children_.empty()) {
      new_root_ = std::make_shared<TrieNodeWithValue<T>>(node_value);
    }
    else {
      new_root_ = std::make_shared<TrieNodeWithValue<T>>(root_->children_, node_value);
    }

    return Trie(std::move(new_root_));
  }

  // 创建一个新节点来当root
  std::shared_ptr<TrieNode> new_root_ = nullptr;

  if(root_ == nullptr) {
    new_root_ = std::make_shared<TrieNode>();
  }
  else {
    // new_root_ = root_->Clone();
    new_root_ = std::shared_ptr<TrieNode>(root_->Clone());
  }
  // 现在新的root节点已经找到，要做的是遍历路径找到

  auto curr = new_root_;
  // 注意这里是递归到叶子节点的前一个
  for(size_t i = 0; i + 1 < key.size(); i++) {
    auto it = curr->children_.find(key[i]);
    if(it == curr->children_.end()) {
      auto new_leaf = std::make_shared<TrieNode>();
      curr->children_.insert({key[i], new_leaf});
      curr = new_leaf;
    }
    else {
      // frist和second都是指针
      // 这个位置把curr->children中对应key的节点直接换成新节点
      std::shared_ptr<TrieNode> new_leaf = it->second->Clone();

      // const节点无法修改！！！
      it->second = new_leaf;
      curr = new_leaf;
    }
  }

  // 现在curr到了最后需要加value的节点
  // 每次只put一个字符的value上去
  char last_c = key.back();
  //这里是把children_更新了
  auto it = curr->children_.find(last_c);
  if(it == curr->children_.end()) {
    // 如果没找到最后一个字符
    auto new_last_leaf = std::make_shared<TrieNodeWithValue<T>>(std::make_shared<T>(std::move(value)));
    curr->children_.insert({last_c, new_last_leaf});
  }
  else {
    std::shared_ptr<T> new_value = std::make_shared<T>(std::move(value));
    // it->second = std::make_shared<TrieNodeWithValue<T>>(it->second->children_, val_p);
    auto new_last_leaf = std::make_shared<TrieNodeWithValue<T>>(it->second->children_, new_value);
    it->second = new_last_leaf;
  }
  
  return Trie(new_root_);
  // You should walk through the trie and create new nodes if necessary. If the node corresponding to the key already
  // exists, you should create a new `TrieNodeWithValue`.
}
// 这里remove的意思是删掉对应的key最后一个节点的value，并且将该类从TrieNodeWithValue换成TrieNode。
// 如果该节点已经不存在子节点，直接删掉该节点即可。
// 记住Clone是unique_ptr！！！
auto Trie::Remove(std::string_view key) const -> Trie {
  // throw NotImplementedException("Trie::Remove is not implemented.");

  // 找到对应的key，然后将其删除，和get做对应
  if(root_ == nullptr) {
    return Trie(root_); 
  }

  auto curr0 = root_;
  
  // 防止没有该key
  for(size_t i = 0; i < key.size(); i ++ ) {

    auto it = curr0->children_.find(key[i]);
    // 如果没找到某一节，说明trie树上没有该key
    if(it == curr0->children_.end()) {
      return Trie(root_);
    }
    curr0 = it->second;
  }
 
  // 此时没有return说明在trie树中有这个单词
  // unique_ptr 不能复制，只能移动
  std::shared_ptr<TrieNode> new_root_ = nullptr;


  new_root_ = std::shared_ptr<TrieNode>(root_->Clone());
  auto curr = new_root_;


  for(size_t i = 0; i + 1 < key.size(); i++) {
    auto it = curr->children_.find(key[i]);

    if(it == curr->children_.end()) {
      return Trie(root_);
    }
    else {
        // curr->children_ = new_leaf;
        std::shared_ptr<TrieNode> new_leaf = std::shared_ptr<TrieNode>(it->second->Clone());
        it->second = new_leaf;
        curr = new_leaf;
       }
  }

  auto last_curr = curr;
  auto it = curr->children_.find(key.back());

  // std::shared_ptr<TrieNode>(root_->Clone());

  curr = std::shared_ptr<TrieNode>(it->second->Clone()); //这一句有问题

  // 现在的curr是最后一个节点
  // 如果现在curr没有儿子
  if(!curr->children_.empty()) {
    // last_curr->children_->second = nullptr;
    last_curr->children_.erase(key.back());
  }
  //有儿子，转变为TrieNode
  else {
    // 如果现在curr有儿子，则将curr转变为TrieNode形式

    // auto new_last_leaf = std::make_shared<TrieNodeWithValue<T>>(it->second->children_, new_value);
    // auto new_leaf = std::make_shared<TrieNode>();
    // new_leaf->children_ = curr->children_;
    // last_curr->children_[key.back()] = new_leaf;

    auto new_leaf = std::make_shared<TrieNode>(it->second->children_);
    last_curr->children_[key.back()] = new_leaf;
  }

  return Trie(new_root_);

  // You should walk through the trie and remove nodes if necessary. If the node doesn't contain a value any more,
  // you should convert it to `TrieNode`. If a node doesn't have children any more, you should remove it.
}

// Below are explicit instantiation of template functions.
//
// Generally people would write the implementation of template classes and functions in the header file. However, we
// separate the implementation into a .cpp file to make things clearer. In order to make the compiler know the
// implementation of the template functions, we need to explicitly instantiate them here, so that they can be picked up
// by the linker.

template auto Trie::Put(std::string_view key, uint32_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint32_t *;

template auto Trie::Put(std::string_view key, uint64_t value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const uint64_t *;

template auto Trie::Put(std::string_view key, std::string value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const std::string *;

// If your solution cannot compile for non-copy tests, you can remove the below lines to get partial score.

using Integer = std::unique_ptr<uint32_t>;

template auto Trie::Put(std::string_view key, Integer value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const Integer *;

template auto Trie::Put(std::string_view key, MoveBlocked value) const -> Trie;
template auto Trie::Get(std::string_view key) const -> const MoveBlocked *;

}  // namespace bustub
