このプログラムはUECS対応センサユニットA型の一部構成部品が入手不可能になったため、一部機能を削除する代わりに部品無しで動作できるようにしたものです。
動作プラットフォームはArduino MEGAです。

GPS、時計、SDカード記録機能を削除する代わりにUECS用の情報を発信するセンサとして利用できます。
データの記録はPCあるいはクラウドを使用してください。

UECS対応センサユニットA型の詳細な資料は以下にあります。

https://www.naro.go.jp/publicity_report/publication/pamphlet/tech-pamph/135291.html

使い方
"SENSOR-TYPEA-Simple_Release1.1_NOGPS"のフォルダ(名前を変えないこと)以下のファイルをPCにダウンロードします。

コンパイルには以下のファイルに含まれるライブラリも必要です。

https://www.naro.go.jp/publicity_report/publication/files/Attached_zip_files_1.zip

Arduino IDEで"SENSOR-TYPEA-Simple_Release1.1_NOGPS.ino"ファイルを読み込み、ハードコーディングされた箇所(UECS-ID)を書き換えた後、コンパイルしてArduino MEGAに書き込んでください。
なお、MACアドレスはW5500に内蔵されたものが自動的に割り当てられます。

