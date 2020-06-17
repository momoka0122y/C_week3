ターミナル上でお絵かきする物。
実行して、以下のコマンドでお絵かきできます。

使用言語:C
構造体の勉強になりました。


コマンド一覧

line　x0, y0, x1, y1
二点間を結ぶ線

square　x0, y0, x1, y1
二点を対角とする長方形

circle x0, y0, r
点を中心とする半径rの円

inside_circle x0, y0, r
点を中心とする半径rの円とその中身

pen　記号
それ以後のペン先が変わる
それ以前は変化しない

color 色
それ以後の色が変わる
それ以前は変化しない

load filename
一般的なopen


paste filename
一般的なpaste
loadと違いそれまで描いた絵にかぶせて書く

doraemon
"load doraemon.txt" と同義
頑張って作ったドラえもんです

save filename
名前をつけて保存

undo

redo

reset
キャンバスを白色に

only_color
背景色を変える色　all_red などで描いた後にこれをやると綺麗なドット絵になります。


doraemon
only_color
って打つと綺麗なドラえもんが見れます。
