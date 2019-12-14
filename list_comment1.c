#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

const int maxlen = 1000;

// A node which belongs to a linear list
// 線形リストの要素となる構造体
struct node 
{
  char *str;
  struct node *next;
};
typedef struct node Node;

struct history{
    Node *begin;
};
typedef struct history History;
// Add another name for struct by typedef
// typedef で Node型として使えるように宣言


// 線形リストの先頭に新しくデータを追加 (push) する関数
// 引数:
// - 現在の線形リストの先頭へのポインタ (begin)
// - 追加するべき文字列 (str)
//
// 出力:
// - 新しく追加されたNodeへのポインタ（新しく先頭になる）
//
void *push_front(History *yamanote, const char *str)
{
  // Create a new element
  // 追加するデータのために構造体とその中身を確保
  Node *p = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str); // 文字列をコピーする (str から s へ)

  // charポインタと次の行き先をメンバに持つ構造体とする
  *(yamanote->begin) = (Node){.str = s, .next = yamanote->begin};////////////////////////////////
 // Now the new element is the first element in the list
}

// 線形リストの先頭からデータを取り除く (pop) する関数
// 引数:
// - 現在の線形リストの先頭へのポインタ (begin)
//
// 出力:
// - 削除後に先頭になるNodeへのポインタ（もともと2番目だったもの）
//

void *pop_front(History *yamanote)
{
  // NULL のnext は当然指定できないので、現在、空の場合はとまる
  assert(yamanote->begin != NULL); // Don't call pop_front() when the list is empty
  Node *p = yamanote->begin->next; // 2番目のアドレスを確保（これをreturnする）

  free(yamanote->begin->str);// 現在先頭の構造体の中身はfree
  free(yamanote->begin);// 構造体自体もfreeする

  yamanote->begin= p;
}

// 線形リストの末尾に新しくデータを追加 (push) する関数
// 引数:
// - 現在の線形リストの先頭へのポインタ (begin)
// - 追加するべき文字列 (str)
//
// 出力:
// - 線形リストの先頭へのポインタ（begin と同じ）
//

void *push_back(History *yamanote, const char *str)
{
  // 現状、空の場合は後ろには追加できないので、前に追加する
  if (yamanote->begin == NULL) {   // If the list is empty
    return push_front(yamanote->begin, str);
  }

  // Find the last element
  Node *p = yamanote->begin;
  // インデックス以外でforやwhile ループを回す方法の一つ
  // 線形リストの場合はp->next で指定された次の構造体へのアドレスを順に辿ればいい
  // 次の要素がNULL になっているものを見つけるまで続ける
  while (p->next != NULL) {
    p = p->next;
  }
  // ループを抜けた時点でpは現在、末尾の構造体を指している
  
  // Create a new element
  // 追加する構造体用のメモリを確保する (このあたりはpush_frontの時と同じ)
  Node *q = (Node *)malloc(sizeof(Node));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  //今回確保したものは末尾にあるので、nextがNULLをさすようにする
  *q = (Node){.str =s, .next = NULL};

  // The new element should be linked from the previous last element
  // pからqにつなぐ
  p->next = q;

}

// 実習1: pop_back の実装
// 線形リストの先頭からデータを取り除く (pop) する関数
void *pop_back(History *yamanote){
  assert(yamanote->begin != NULL);
   Node *p = yamanote->begin; 
    Node *q;
  while (p->next != NULL) {
      q = p;
    p = p->next;// ループを抜けた時点でpは現在、末尾の構造体を指している
  } 
  free(p->str);// 現在先頭の構造体の中身はfree
  free(p);
  q->next =NULL;

}
// 引数:
// - 現在の線形リストの先頭へのポインタ (begin)
//
// 出力:
// - 新しく先頭になるNodeへのポインタ（begin と同じ）
//



Node *remove_all(History *yamanote)
{
  while ((yamanote->begin = pop_front(yamanote->begin))) 
    ; // Repeat pop_front() until the list becomes empty
  return yamanote->begin;  // Now, begin is NULL
}

int main()

{
  History *yamanote =(History*)malloc(sizeof(History));
  yamanote->begin=NULL

  // Read all lines from stdin and store them in the list
  char buf[maxlen];
  while (fgets(buf, maxlen, stdin)) {
    //begin = push_front(begin, buf);
    push_back(yamanote, buf); // Try this instead of push_front()
  }

    pop_back(yamanote);  // What will happen if you do this?
  //begin = pop_back(begin);   // What will happen if you do this?
 
  //begin = remove_all(begin); // What will happen if you do this?

  // Print all the strings stored in the list
  // ポインタを辿ってfor文を回している。先頭アドレスからスタートして、アドレスをp->next がさす次の構造体へ更新し、NULL (終端) に当たるまで続ける
  for (const Node *p = yamanote->begin; p != NULL; p = p->next) {
    printf("%s", p->str);
  }
  
  return 0;
}
