#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h> // for error catch
#include <math.h>//あとやること　塗りつぶしの円。名前をつけて保存。色を変える。ペン先を変える。←どっちもkコマンドNORMAL

// Structure for canvas
typedef struct
{
  int width;
  int height;
  char **canvas;
  char pen;
} Canvas;
///////////////////////

typedef struct command{
  char *buf;
  struct command *next;
}Command;

////////////////////////
// Structure for history (2-D array)
typedef struct{
  size_t bufsize;
  Command *begin;
  size_t hsize;
} History;

// functions for Canvas type
Canvas *init_canvas(int width, int height, char pen);
void reset_canvas(Canvas *c);
void print_canvas(FILE *fp, Canvas *c);
void free_canvas(Canvas *c);

// display functions
void rewind_screen(FILE *fp,unsigned int line);
void clear_command(FILE *fp);
void clear_screen(FILE *fp);

// enum for interpret_command results
typedef enum res{ EXIT, NORMAL, COMMAND} Result;

int max(const int a, const int b);
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1);
void draw_circle(Canvas *c, const int x0, const int y0, const int r);
Result interpret_command(const char *command, History *his,History *redo, Canvas *c);
void save_history(const char *filename, History *his);

////////
void push_back(History *history, const char *str);
void push_front(History *history, const char *str);
void pop_back(History *history);
void pop_front(History *history);




int main(int argc, char **argv)
{
  //for history recording
  const int bufsize = 1000;
  History history = (History){.bufsize = bufsize, .hsize = 0 ,.begin=NULL};
  History redo= (History){.bufsize = bufsize, .hsize = 0 ,.begin=NULL};


  int width;
  int height;
  if (argc != 3){
    fprintf(stderr,"usage: %s <width> <height>\n",argv[0]);
    return EXIT_FAILURE;
  } else{
    long w = strtol(argv[1],NULL,10);
    if(errno == ERANGE){
      fprintf(stderr, "%s: %s\n",argv[1],strerror(errno));
      return EXIT_FAILURE;
    }
    long h = strtol(argv[2],NULL,10);
    if(errno == ERANGE){
      fprintf(stderr, "%s: %s\n",argv[2],strerror(errno));
      return EXIT_FAILURE;
    }
    width = (int)w;
    height = (int)h;    
  }
  char pen = '*';

  FILE *fp;
  char buf[history.bufsize];
  fp = stdout;
  Canvas *c = init_canvas(width,height, pen);

  fprintf(fp,"\n"); // required especially for windows env
  while (1) {
    size_t hsize = history.hsize;


    size_t bufsize = history.bufsize;
    print_canvas(fp,c);
    printf("%zu > ", hsize);
    fgets(buf, bufsize, stdin);


    const Result r = interpret_command(buf, &history,&redo,c);
    if (r == EXIT) break;
    if (r == NORMAL) {


      push_back(&history, buf);

      history.hsize++;
      
    }
    for (const Command *p = history.begin; p != NULL; p = p->next) {
    printf("%zu  %s",history.hsize, p->buf);
  }
  for (const Command *p = redo.begin; p != NULL; p = p->next) {
    printf("\e[31m%zu %s\e[0m",redo.hsize, p->buf);
  }

    rewind_screen(fp,2+history.hsize); // command results
    clear_command(fp); // command itself
    rewind_screen(fp, height+2); // rewind the screen to command input
    //clear_command(fp); // clear the previous command
  }

  clear_screen(fp);
  free_canvas(c);
  fclose(fp);

  return 0;
}

Canvas *init_canvas(int width,int height, char pen)
{
  Canvas *new = (Canvas *)malloc(sizeof(Canvas));
  new->width = width;
  new->height = height;
  new->canvas = (char **)malloc(width * sizeof(char *));

  char *tmp = (char *)malloc(width*height*sizeof(char));
  memset(tmp, ' ', width*height*sizeof(char));
  for (int i = 0 ; i < width ; i++){
    new->canvas[i] = tmp + i * height;
  }
  
  new->pen = pen;
  return new;
}

void reset_canvas(Canvas *c)
{
  const int width = c->width;
  const int height = c->height;
  memset(c->canvas[0], ' ', width*height*sizeof(char));
}


void print_canvas(FILE *fp, Canvas *c)
{
  const int height = c->height;
  const int width = c->width;
  char **canvas = c->canvas;
  
  // 上の壁
  fprintf(fp,"+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");

  // 外壁と内側
  for (int y = 0 ; y < height ; y++) {
    fprintf(fp,"|");
    for (int x = 0 ; x < width; x++){
      const char c = canvas[x][y];
      fputc(c, fp);
    }
    fprintf(fp,"|\n");
  }
  
  // 下の壁
  fprintf(fp, "+");
  for (int x = 0 ; x < width ; x++)
    fprintf(fp, "-");
  fprintf(fp, "+\n");
  fflush(fp);
}

void free_canvas(Canvas *c)
{
  free(c->canvas[0]); //  for 2-D array free
  free(c->canvas);
  free(c);
}

void rewind_screen(FILE *fp,unsigned int line)
{
  fprintf(fp,"\e[%dA",line);
}

void clear_command(FILE *fp)
{
  fprintf(fp,"\e[2K");
}

void clear_screen(FILE *fp)
{
  fprintf(fp, "\e[2J");
}

int max(const int a, const int b)
{
  return (a > b) ? a : b;
}
void draw_line(Canvas *c, const int x0, const int y0, const int x1, const int y1)
{
  const int width = c->width;
  const int height = c->height;
  char pen = c->pen;
  
  const int n = max(abs(x1 - x0), abs(y1 - y0));
  c->canvas[x0][y0] = pen;
  for (int i = 1; i <= n; i++) {
    const int x = x0 + i * (x1 - x0) / n;
    const int y = y0 + i * (y1 - y0) / n;
    if ( (x >= 0) && (x< width) && (y >= 0) && (y < height))
      c->canvas[x][y] = pen;
  }
  //printf("1 line drawn\n");
}
void draw_circle(Canvas *c, const int x0, const int y0, const int r)
{
  const int width = c->width;
  const int height = c->height;
  char pen = c->pen;

  for (int x=0;x<width;x++){
    for (int y=0 ; y<height;y++){
      if ((int) sqrt((double)((x-x0)*(x-x0)+4*(y-y0)*(y-y0)))==r){
        c->canvas[x][y] = pen;
      }
    }
  }


  //printf("1 line drawn\n");
}

void save_history(const char *filename, History *history)
{
  const char *default_history_file = "history.txt";
  if (filename == NULL)
    filename = default_history_file;
  
  FILE *fp;
  if ((fp = fopen(filename, "w")) == NULL) {
    fprintf(stderr, "error: cannot open %s.\n", filename);
    return;
  }
  

  for (const Command *p = history->begin; p != NULL; p = p->next) {
    fprintf(fp,"%s", p->buf);
  }

  //printf("saved as \"%s\"\n", filename);

  fclose(fp);
}

Result interpret_command(const char *command, History *his,History *redo, Canvas *c)
{
  
  char buf[his->bufsize];
  strcpy(buf, command);

  if(strlen(buf)==0){
    printf("error: unknown command.\n");
    return COMMAND;
  }
  buf[strlen(buf) - 1] = 0; // remove the newline character at the end
  const char *s = strtok(buf, " ");

  // The first token corresponds to command


  if (strcmp(s, "line") == 0) {
    int x0, y0, x1, y1;
    x0 = 0; y0 = 0; x1 = 0; y1 = 0; // initialize
    char *b[4];
    for (int i = 0 ; i < 4; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
        printf("the number of point is not enough.\n");
        return COMMAND;
      }
    }
    x0 = strtol(b[0],NULL,10);
    y0 = strtol(b[1],NULL,10);
    x1 = strtol(b[2],NULL,10);
    y1 = strtol(b[3],NULL,10);

    draw_line(c,x0, y0, x1, y1);
    return NORMAL;
  }
  if (strcmp(s, "square") == 0) {
    int x0, y0, x1, y1;
    x0 = 0; y0 = 0; x1 = 0; y1 = 0; // initialize
    char *b[4];
    for (int i = 0 ; i < 4; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
        printf("the number of point is not enough.\n");
        return COMMAND;
      }
    }
    x0 = strtol(b[0],NULL,10);
    y0 = strtol(b[1],NULL,10);
    x1 = strtol(b[2],NULL,10);
    y1 = strtol(b[3],NULL,10);

    draw_line(c,x0, y0, x0, y1);
    draw_line(c,x0, y0, x1, y0);
    draw_line(c,x1, y1, x0, y1);
    draw_line(c,x1, y1, x1, y0);
    return NORMAL;
  }
  if (strcmp(s, "circle") == 0) {
    int x0, y0, r;
    x0 = 0; y0 = 0; r=0; // initialize
    char *b[3];
    for (int i = 0 ; i < 3; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
        printf("the number of point is not enough.\n");
        return COMMAND;
      }
    }
    x0 = strtol(b[0],NULL,10);
    y0 = strtol(b[1],NULL,10);
    r = strtol(b[2],NULL,10);

    draw_circle(c,x0, y0, r);
    printf("1 circle drawn\n");
    return NORMAL;
  }

    if (strcmp(s, "inside_circle") == 0) {
    int x0, y0, r;
    x0 = 0; y0 = 0; r=0; // initialize
    char *b[3];
    for (int i = 0 ; i < 3; i++){
      b[i] = strtok(NULL, " ");
      if (b[i] == NULL){
        printf("the number of point is not enough.\n");
        return COMMAND;
      }
    }
    x0 = strtol(b[0],NULL,10);
    y0 = strtol(b[1],NULL,10);
    r = strtol(b[2],NULL,10);

    draw_inside_circle(c,x0, y0, r);
    printf("1 circle drawn\n");
    return NORMAL;
  }

    if (strcmp(s, "load") == 0) {
       FILE *fp;
       char str[1024];

       fp = fopen("history.txt","r");

       if(fp==NULL){
         printf("ファイルオープン失敗\n");
         return COMMAND;
         }
         reset_canvas(c); 
         char buf[10000];
         int i=0;///////////////////////////
         History load_history =(History){.bufsize = 1000, .hsize = 0 ,.begin=NULL};
         while (fgets(buf,10000, fp)) {
           interpret_command(buf, his,redo, c);
           push_back(&load_history,buf);
           load_history.hsize++;
         }
         *his=load_history;
    }
  
  if (strcmp(s, "doraemon") == 0) {
       FILE *fp;
       char str[1024];
       fp = fopen("doraemon.txt","r");

       if(fp==NULL){
         printf("ファイルオープン失敗\n");
         return COMMAND;
         }
         reset_canvas(c); 
         char buf[10000];
         int i=0;///////////////////////////
         History load_history =(History){.bufsize = 1000, .hsize = 0 ,.begin=NULL};
         while (fgets(buf,10000, fp)) {
           interpret_command(buf, his,redo, c);
           push_back(&load_history,buf);
           load_history.hsize++;
         }
         *his=load_history;
    }

  if (strcmp(s, "save") == 0) {
    s = strtok(NULL, " ");
    save_history(s, his);
    printf("saved as \"%s\"\n",(s==NULL)?"history.txt":s);
    return COMMAND;
  }

  if (strcmp(s, "undo") == 0) {
    
    if (his->hsize != 0){
     reset_canvas(c); 

    const Command *xxx=his->begin; 
    while (xxx->next!=NULL ){
      xxx=xxx->next;
    }
    push_front(redo, xxx->buf);

     pop_back(his);
     const Command *z = his->begin;

     while (z != NULL) {
       interpret_command(z->buf, his,redo, c);
       z = z->next;
      }

      his->hsize--;
    }
    return COMMAND;
  }

if (strcmp(s, "redo") == 0) {
  if( redo->begin != NULL){
    push_back(his,redo->begin->buf);
    pop_front(redo);
    const Command *z = his->begin;

    while (z != NULL) {
      interpret_command(z->buf, his,redo, c);
      z = z->next;
      }
      his->hsize++;
  }
  return COMMAND;
}








  if (strcmp(s, "quit") == 0) {
    return EXIT;
  }

  printf("error: unknown command.\n");

  return COMMAND;
}

/////////////////////////////
void push_front(History *history, const char *str)
{
  // Create a new element
  // 追加するデータのために構造体とその中身を確保
  Command *p = (Command *)malloc(sizeof(Command));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str); // 文字列をコピーする (str から s へ)
  // charポインタと次の行き先をメンバに持つ構造体とする
  Command *q = (Command*)malloc(sizeof(Command));
  *q = (Command){.buf = s, .next = history->begin};//一つ目でしか使わないからこの時.nextはNULLになる。
  history->begin = q;
 // Now the new element is the first element in the list
}
void push_back(History *history, const char *str)
{
  // 現状、空の場合は後ろには追加できないので、前に追加する
  if (history->begin == NULL) {   // If the list is empty
    return push_front(history, str);
  }
  // Find the last element
  Command *p = history->begin;
  while (p->next != NULL) {
    p = p->next;
  }
  // ループを抜けた時点でpは現在、末尾の構造体を指している
  // Create a new element
  // 追加する構造体用のメモリを確保する (このあたりはpush_frontの時と同じ)
  Command *q = (Command *)malloc(sizeof(Command));
  char *s = (char *)malloc(strlen(str) + 1);
  strcpy(s, str);

  //今回確保したものは末尾にあるので、nextがNULLをさすようにする
  *q = (Command){.buf =s, .next = NULL};
  // The new element should be linked from the previous last element
  // pからqにつなぐ
  p->next = q;

}

void pop_back(History *history){
  if(history->begin != NULL){
   Command *p = history->begin; 
    Command *q;
  while (p->next != NULL) {
      q = p;
    p = p->next;// ループを抜けた時点でpは現在、末尾の構造体を指している
  } 
  free(p->buf);// 現在先頭の構造体の中身はfree
  free(p);
  q->next =NULL;
  }


}

void pop_front(History *history)
{
  // NULL のnext は当然指定できないので、現在、空の場合はとまる
  //assert(history->begin != NULL); // Don't call pop_front() when the list is empty 
  //なぜかassertが使えないのでいつかTAさんに聞いて見る
  if(history->begin != NULL){
  Command *p = history->begin->next; // 2番目のアドレスを確保（これをreturnする）

  free(history->begin->buf);// 現在先頭の構造体の中身はfree
  free(history->begin);// 構造体自体もfreeする

  history->begin= p;
  }
}