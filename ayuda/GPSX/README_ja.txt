
Playsation2 controller interface library for Arduino.

(Japanese document revised Oct. 25, 2008)


C/C++を書く初の試みにつき、アドバイス＆フィードバック歓迎致します。
gyokimae at ホットメール・ドットコムまで。


*********
* 目次  *
*********

1. ライブラリのインストール

2. ハードウェアのピン配置

3. 関数リファレンス

4. 製作中にハマった箇所

5. その他



==============================================================================
1. ライブラリのインストール
==============================================================================

Like any other Arduino library, place the files under
<arduino path>/hardware/libraries/
and restart the Arduino IDE.



==============================================================================
2. ハードウェアのピン配置
==============================================================================

PS2コントローラのピン配置は、各種サイトで参照できます。
回路図shcematic.bmpを添付しますが、コントローラ側のどの線がどのピンに該当する
かは、これらサイトをご参照下さい。

以下のサイトは通信プロトコルについても詳細に説明されており、割と参考になりました。
（ほとんどサイトが、PS1のタイミングにしか言及していないので）
http://www.curiousinventor.com/guides/ps2


ライブラリが好むピン配置は、現在 GPSX.c 内にてハードコードされています。
不都合がある場合は各自修正して下さい。


Arduino Pin No. | PSX Controller Pin No.
----------------+------------------------
Pin 7           | Pin 1 Data
Pin 6           | Pin 2 Command
                | Pin 3 9V for Motor (Optional)
                | Pin 4 GND
                | Pin 5 VCC (3.3V)
Pin 5           | Pin 6 Attention (PAD1)
Pin 4           | Pin 7 Clock
                | Pin 8 N/C
Pin 3           | Pin 9 ACK

Pin 2           | Pin 6 Attention (PAD2)


＊コントローラを一つしか使わない場合、Arduino側２番ピンは使用する
　必要はありません。
　それ以外のピンは、コントローラ１＆２で共有となります（Arduino側の
　同じピンに接続します）
　なぜこれが可能かといいますと、都度送られたコマンドに対して、その時
　Attentionピンがローになっているコントローラしか反応しない設計になって
　いるためです。

＊９Ｖは、振動機能を使用する場合にのみ接続して下さい。

　おすすめのコンフィギュレーションは、
　　振動機能を使用する場合
　　　→ Arduinoの動作に、9V ACアダプタを使用。
　　　　コントローラの電源は、Arduino上のVinピンから供給

　　振動機能を使用しない場合
　　　→ コントローラのピン３をどこにも接続しない。
　　　　Arduinoは任意の電源電圧で動作


なお、9Ｖ以外の電源をコントローラ側３番ピンに接続すると誤動作
（ボタン状態の読み取りミス）の原因となるようです。


以下のピンは1kΩのプルアップ抵抗を必要とします。

 * コントローラ1,2のAttentionピン(Arduino側２＆５)
 * Dataピン（Arduino側７番ピン

                  Vcc 5V
                    |
                 (1kOhm)
                    |
   (Arduino Pin)----+----(Controller)


  その他のピンは、二つのコントローラ間で共有されます。


コントローラをArduinoに接続する方法は何通りか考えられます。
手近なコントローラのコネクタを切り落としてケーブルを剥き出しにする方法も
ありますが、ジャンク機器などからコネクタを入手した方が、コントローラが
使いまわせていいかも知れません。

作者は、めっきり使用しなくなったPSコン⇔USBの変換器がありましたので、
これを分解してコネクタ二つ入手しました。


ちなみに、Arduinoの内蔵プルアップ抵抗(数十kΩ）を使用した場合は正しく
データが読み取れませんでした。



==============================================================================
3. 関数リファレンス
==============================================================================

- 基本的にいずれも関数も、ターゲットとなるコントローラの番号を指定する必要が
　あります。これには、PSX_PAD1, PSX_PAD2の二つのマクロを使用できます。

- ボタンの押し下げ状況は、PSX.updateState(padNo)呼び出し時にのみ更新されます。

- 振動モータの回転状態を変更するには、２つのステップを経る必要があります。
  まず、PSX.motor()関数で状態を設定します。変更した設定は次回PSX.updateState()
  関数呼び出し時に反映されます。



+------------------------+
| PSX.updateState(padNo) |
+------------------------+
  キー押し下げ状態を更新

     padNo  = PSX_PAD1 | PSX_PAD2


    上記関数を呼び出し後、以下のマクロを使用してボタンの状態を調べます。



      *** キーが現在押し下げられているか ***
          trueなら適当なintが、falseならゼロが返ります。

      IS_DOWN_LEFT(padNo)
      IS_DOWN_DOWN(padNo)
      IS_DOWN_RIGHT(padNo)
      IS_DOWN_UP(padNo)
      IS_DOWN_START(padNo)
      IS_DOWN_STICK_RIGHT(padNo)
      IS_DOWN_STICK_LEFT(padNo)
      IS_DOWN_SELECT(padNo)

      IS_DOWN_SQUARE(padNo)
      IS_DOWN_CROSS(padNo)
      IS_DOWN_CIRCLE(padNo)
      IS_DOWN_TRIANGLE(padNo)
      IS_DOWN_R1(padNo)
      IS_DOWN_L1(padNo)
      IS_DOWN_R2(padNo)
      IS_DOWN_L2(padNo)


      *** 前回updateState()関数呼び出し時からキーが押されたか
          trueなら適当なintが、falseならゼロが返ります。

      PRESSED_LEFT(padNo)
      PRESSED_DOWN(padNo)
      PRESSED_RIGHT(padNo)
      PRESSED_UP(padNo)
      PRESSED_START(padNo)
      PRESSED_STICK_RIGHT(padNo)
      PRESSED_STICK_LEFT(padNo)
      PRESSED_SELECT(padNo)

      PRESSED_SQUARE(padNo)
      PRESSED_CROSS(padNo)
      PRESSED_CIRCLE(padNo)
      PRESSED_TRIANGLE(padNo)
      PRESSED_R1(padNo)
      PRESSED_L1(padNo)
      PRESSED_R2(padNo)
      PRESSED_L2(padNo)

      RELEASED_LEFT(padNo)
      RELEASED_DOWN(padNo)
      RELEASED_RIGHT(padNo)
      RELEASED_UP(padNo)
      RELEASED_START(padNo)
      RELEASED_STICK_RIGHT(padNo)
      RELEASED_STICK_LEFT(padNo)
      RELEASED_SELECT(padNo)


      *** 前回updateState()関数呼び出し時からキーが離されたか
          trueなら適当なintが、falseならゼロが返ります。

      RELEASED_SQUARE(padNo)
      RELEASED_CROSS(padNo)
      RELEASED_CIRCLE(padNo)
      RELEASED_TRIANGLE(padNo)
      RELEASED_R1(padNo)
      RELEASED_L1(padNo)
      RELEASED_R2(padNo)
      RELEASED_L2(padNo)


      *** アナログスティックの値読み取り
          0 から 255 (0x00 から 0xFF) の値が返ります。

      ANALOG_RIGHT_X(padNo)
      ANALOG_RIGHT_Y(padNo)
      ANALOG_LEFT_X(padNo)
      ANALOG_LEFT_Y(padNo)



+----------------------------------------------------+
| PSX.motorEnable(padNo, Motor1Enable, Motor2Enable) |
+----------------------------------------------------+
  振動モータのオン・オフ

     padNo         = PSX_PAD1 | PSX_PAD2

     Motor1Enable  = MOTOR1_ENABLE | MOTOR1_DISABLE

     Motor2Enable  = MOTOR2_ENABLE | MOTOR2_DISABLE



+----------------------------------------------------+
| PSX.motor(padNo, Motor1OnOff, uint8_t Motor2Speed) |
+----------------------------------------------------+
  モータの回転速度変更

     padNo         = PSX_PAD1 | PSX_PAD2

     Motor1OnOff  = MOTOR1_ON | MOTOR1_OFF

     Motor2Speed  = uint8_t between 0 to 255 (0x00 to 0xFF)


  この関数は、新しい回転速度の値をセットするだけです。
  変更は、次回PSX.updateState()関数呼び出し時に反映されます。

　モータ１はオン・オフのみ設定可能、
　モータ２は回転速度を設定できます。



+-----------------------------+
| PSX.mode(padNo, mode, lock) |
+-----------------------------+
 アナログ/デジタルモードの切り替え

     padNo  = PSX_PAD1 | PSX_PAD2

     mode   = MODE_DIGITAL | MODE_ANALOG

     lock   = MODE_UNLOCK | MODE_LOCK


　ロックを有効にした場合、コントローラ上のAnalogボタンでユーザが任意に
　モードを切り替えることができなくなります。



==============================================================================
4. 製作中にハマった箇所
==============================================================================

ＰＳ１＆２のコントローラは、ホストからデータを１バイト受信する度にACKピンを一瞬
ローにすることが各種サイトで解説されています。
しかし、ＰＳ１はともかく２のコントローラは、このACK時間があまりに短かすぎて、
Arduinoのクロック速度でポーリングすることができませんでした。
この辺りで数日ハマった後、ACKピンによる応答を無視して、適当な間合いでバイトを
送信し続けたところすんなり動作しました。
少なくとも、手元にあるSony純正のDualShock2およびアナログ振動パッド(HORI)では
もっとも安定しているように見えたタイミングに調整してあります。
（残念ながらロジアナ等の計測器を所有していないもので）

ライブラリの現在のバージョンは、単純に９バイト送信して応答をみています。
実際にはコントローラから返る２バイト目に残りの受信バイト長が含まれているのですが…
現在のところ処理は省略


==============================================================================
5. その他
==============================================================================

* 現在、動作タイミングはPS2にのみ合わせて調整してあります。

* コントローラ２つの同時使用は作者側では未検証ですが、理論上は問題ないはずです。


■付属サンプルについて（日本語ドキュメントのみ）

ライブラリが提供する一通りの機能が検証できるようになっています。

* 起動時に、コントローラはアナログモードのままロックされており、パッド上のボタンでは
　変更できないようになっています。

* 四角ボタンとＸボタンは、パッドのアナログ/デジタルモードを切り替えます。
　この時点でロックは解除され、Analogボタンでユーザが任意にモード切替可能です。

* ○ボタン押し下げ中は、モータ１が回転します。

* 三角ボタン押し下げ中、モータ２が回転します。
　また、このとき左スティックのＸ軸が回転速度に割り当てられています。
