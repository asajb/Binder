**Binder**

To facilitate their exam preparation, a group of computer science students decided to systematize their note-taking method using a binder. For this method to be practical and truly systematic, the students determined that the binders they use must meet certain basic requirements.

The format of note content should not be predetermined (after all, some students take notes by hand in class, while others prefer to use electronic devices), but should be consistent within a single binder. Additionally, each note should be visibly marked with a (unique within a single binder) tab to facilitate searching.

You need to implement a binder class template available in the cxx namespace with the following declaration:

```cpp
namespace cxx {
  template <typename K, typename V> class binder;
}
```

Where the key type (tab) K has value semantics, meaning it has a default parameterless constructor, copy constructor, move constructor, assignment operators, and destructor available. Type K has a linear order defined and all comparisons can be performed on objects of this type. About type V (note content), we can only assume it has a public copy constructor and public destructor.

Students willingly share notes. Since they are generally thrifty, they prefer sharing the same binder rather than creating a new physical copy. However, the situation changes when one user of a shared binder decides to make changes to it. In this situation, to avoid disrupting their colleagues' studies, the student must go to the copy shop and create their own copy of the binder.

Formally, the container should implement copy-on-write semantics.

Copy-on-write is an optimization technique widely used, among others, in data structures from the Qt library and formerly in std::string implementations. Its basic idea is that when we create a copy of an object (in C++ using the copy constructor or assignment operator), it shares all internal resources (which may be stored in a separate object on the heap) with the source object. This state continues until one of the copies needs to be modified. Then the modified object creates its own copy of the resources on which it performs the modification.

To enable efficient collection and modification of notes, the binder class should provide the operations described below. For each operation, its time complexity is given assuming no copy needs to be made. The time complexity of copying a binder is O(n log n), where n denotes the number of stored notes. All operations must provide at least a strong exception safety guarantee, and where possible and desirable (for example, move constructor and destructor), they must not throw exceptions.

**Constructors:** parameterless creating an empty binder, copy and move constructors. Complexity O(1).

```cpp
binder();
binder(binder const &);
binder(binder &&);
```

**Assignment operator** that takes the argument by value. Complexity O(1) plus the time to destroy the overwritten object.

```cpp
binder & operator=(binder);
```

**Method insert_front** inserts a note with the given tab at the beginning of the binder. To maintain tab uniqueness, a new note cannot be inserted with a tab currently used in the binder – in such a situation, the method throws std::invalid_argument. Complexity O(log n).

```cpp
void insert_front(K const &k, V const &v);
```

**Method insert_after** allows placing a note with the given tab k directly after the note with tab prev_k. The method works similarly to insert_front, but also throws std::invalid_argument when the tab prev_k cannot be found in the binder. Complexity O(log n).

```cpp
void insert_after(K const &prev_k, K const &k, V const &v);
```

**Parameterless method remove** removes the first note from the binder. If the binder is empty, it throws std::invalid_argument. Complexity O(log n).

```cpp
void remove();
```

**Single-parameter method remove** removes the note with the given tab from the binder. If there is no such note in the binder, it throws std::invalid_argument. Complexity O(log n).

```cpp
void remove(K const &);
```

**Single-parameter methods read** return a reference to the note marked with the given tab. In the non-const version, the returned reference should allow modifying the note. A modifying operation on the binder may invalidate the returned reference. If there is no note with the given tab in the binder, the method throws std::invalid_argument. Complexity O(log n).

```cpp
V & read(K const &);
V const & read(K const &) const;
```

**Method size** returns the number of notes in the binder. Complexity O(1).

```cpp
size_t size() const;
```

**Method clear** empties the binder (preparing it for the next semester), i.e., removes all notes from it. Complexity O(n).

```cpp
void clear();
```

Additionally, to enable quick reading of all notes (for example, during review before an exam), the binder should allow efficient page-flipping, that is, provide:

**Iterator const_iterator** and methods cbegin, cend, as well as operators working on the iterator: assignment (=), comparison (== and !=), increment (both prefix and postfix operator ++), value access (* and ->) allowing viewing notes in the order of their occurrence in the binder. If the binder has not been copied, the iterator remains valid. The iterator must satisfy the std::forward_iterator concept. All operations in O(1) time. The iterator is only for viewing notes and cannot be used to modify the binder, so it behaves like const_iterator from the standard library.

In finalizing the consistent binder format, students decided it does not need to provide methods other than those listed above (such as tab searching that doesn't throw exceptions – at most, a colleague without knowledge of them will have more difficulty quickly searching through a laboriously created binder). Your task is to help this group of students by creating an implementation of binders that meets all the mentioned characteristics.

Where possible and justified, methods should be marked with const and noexcept qualifiers.

A binder class object should store only one copy of each inserted tab and note.

The binder class should be exception-transparent, meaning it should propagate all exceptions thrown by functions it calls and operations on its members, and the observable state of the object should not change. In particular, failed modifying operations must not invalidate iterators.

The solution will be compiled with the command:

```
g++ -Wall -Wextra -O2 -std=c++20 *.cpp
```

The solution should be contained in the file binder.h.
