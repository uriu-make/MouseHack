Linuxでsrc/shortcut.hのDEVICE_NAMEで定義されているマウスのイベントを別のイベントに変換します。\
デバイスの名前は
```
$ cat /proc/bus/input/devices
```
を実行し、出力のN: Name=""を参照してください。\
ショートカットとして使いたいキーやボタン、マウスの操作はioctlで指定する必要があります。\
マウスの接続がない場合、プログラムはマウスが接続されるまで待機し、接続されるとイベントの置換が始まります。また、切断されるとマウスの接続を待機する状態に戻ります。\
このプログラムでは\
・EXTRAボタンをAlt+Tabに変換し、離しても1秒程度Altを有効にする\
・SIDEボタンをCtrl+Alt+0に変換\
・ミドルクリックした状態でマウスを上下に移動で拡大/縮小\
・ミドルクリックした状態でマウスを左右に移動で進む/戻る\
・ミドルクリックしたあとマウスの操作をせず離すと拡大率を初期化\
の機能が用意されています。\
追加のボタンを設定する場合にはsrc/MouseHack.cppの60行目にあるswitch caseと111行目以降の関数群の中から該当するボタンの項目のコメントアウトを外し、src/shortcut.cpp内の該当する関数を編集してください。関数名は"ボタンの名前"_func(...)のように定義されます。また、関数内でさらにマウスの入力を要求する場合、resultに関数の戻り値を代入することを推奨します。これはマウスが接続されているかを確認するための手順です。
```sh
$ sudo make install
```
でビルドしてSystemdに登録できます。\
機能の変更などを行う場合は
```sh
$ sudo systemctl stop mousehack
```
でプログラムを停止してください。