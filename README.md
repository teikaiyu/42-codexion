終了条件：
	- 誰か一人がburnout（前回コンパイル開始からtim_to_burnout msいないに次のコンパイルを開始できなかった）
	- 全員がnumber_of_compiles_required回以上コンパイルし終えた

Coffman条件：
	相互排他（Mutual Exclusion）：リソースを一度に１つのプロセスしか使用できない状態
	保持しながら待機（Hold and Wait）：あるリソースを確保して持ったまま、別のプロセスのリソース解放を待っている状態
	横取り不可（No Preemption）：他のプロセスが占有しているリソースを、強制的に奪い取ることができない性質
	循環待機（Circular Wait）：複数のプロセスが環状になり、お互いが必要としているリソースを持ち合っている状態

1. 排他制御(mutex) — 共有資源(dongle)を同時に二人が持たないようにする
2. 効率的な待機(condition variable) — busy-loopせず、必要な時だけ起こされる
3. デッドロック回避 — 循環待ち（Coffman条件の一つ）を構造的に潰す
4. 公平性・starvation回避 — 誰か一人だけがずっと後回しにされ続ける状況を防ぐ（fifo/edfスケジューラ）
5. 正確なタイミング制御 — burnout検知を10ms以内に行う
6. リソース管理 — mallocしたものは全部free、mutexもcond dextroyまで面倒見る

制約条件
- 42のNorm準拠必須
- セグフォ・ダブルフリー等の異常終了は絶対NG
- メモリリーク一切禁止
- グローバル変数禁止
- 使っていい外部関数は指定リストのみ


全体設計と中核ロジック
データ設計：「１つのsim構造体に全部集約」
t_sim構造体。全パラメータ・全coder・全dongle・全体の状態（stop_flag等）をここに集約、全スレッドがこの１つの構造体へのポインタを共有する。
t_coderはt_sim *を逆参照で持つ——だから書くcoderスレッドは自分の構造体１つ受け取れば、必要な情報全部（時間パラメータ、他のcoderの状態、stop_flag）にアクセスできる。

中核ロジック１：dongle取得の仕組み
発送の転換点：dongle 争奪戦を１つの巨大なheapで管理するのではなく、dongle 一個一個が自分専用の小さいheap（街行列）を持つ。
理由：課題文に「同じdongleを複数人が要求した時に調停する」と明記——競合はdongle単位。一個のdongleは物理的に隣接する二人のcoderからしか要求されない（円環構造上、n>2なら必ず二人）。
// n == 1, n == 0, n % 2 == 0, n % 2 == 1 それぞれの場合の検証が必要
dongle_acquire関数の流れ：
1. このdongleのheapに自分を登録（key=fifoなら要求時刻、edfならburnout締め切り）
2. 条件チェック：自分がheap先頭かつ　空き　かつ　cooldown明け
3. 条件揃うまでpthread_cond_timewaitで待つ
	（待機上限は「cooldown_until」か「今+50ms」の早い方—誰も起こしてくれない場合でも自分で定期的に目を覚ます保険）
4. 条件揃った瞬間、heapから自分を取り除き、in_use=1にして確保完了

中核ロジック２：デッドロック回避
コンパイルには左右dongleが必要。ここでdining philosophers古典問題のデッドロックが起きうる——全員が左手だけもって右手を持つ状態で円環状に膠着する。
解決策：全員「左→右」の順で取得するが、番号が一番大きいcoderだけ「右→左」で取得する。この一点の非対称性によって、循環待ちの構造が理論的に成立しなくなる。
`c->acquire_left_first = (c->id != sim->n_coders);`
n=1の特殊ケースは、left==rightなので二重ロックにならないよう一回だけ取得する分岐で処理。

中核ロジック３：fifo/edfの切り替え
分岐点はたった一箇所——heapに登録する時のkey計算のみ。
```
if (scheduler == SCH_FIFO)
	key = 要求時刻；
else // SCH_EDF
	key = last_compile_start + time_to_burnout; // 締め切り
```
取得ロジック本体は完全共通。fifoは「早いもの順」、edfは「burnout寸前の人優先」——heapのkeyの意味が変わるだけで、同じ仕組みで両方実現。

中核ロジック４：coderスレッドのライフサイクル
```
coder_routine(無限ループ、stop_flagまで):
	dongle両方確保（acquire_both、失敗=stop検知で終了）
	compile (last_compile_start更新→sleep→カウント+1、全員規定回数か確認)
	dongle両方解放（cooldown開始時刻セット）
	debug(sleep)
	refactor(sleep)
	ループ先頭へ
全sleepは5ms刻みの分割sleepでstop_flagを見に行く——burnout等で全体停止が決まった時、フルの待ち時間を待たされず速やかにスレッドが終了できるようにするため。

中核ロジック５：burnout検知（monitor専用スレッド）
coder自身に「自分がもうすぐ死ぬ」と気づかせる設計は危険（sleep中は判定不可能）。だから別スレッドが外から監視する。
```
monitor_routine(1msごとにループ):
	全coderのlast_compile_startを見て回る
	now - last_compile_start > time_to_burnoutなら
		→ログ出力、stop_flag立てる
```
1msという短い周期にしてるのは、課題要求「burnoutログは実際の発生から10ms以内」に対して十分な安全マージンを持たせるため。

中核ロジック６：終了判定の二重性
- burnout: monitorがstop_flagを立てる
- 全員完了: 各coderがcompile完了のたびに、自分のカウンタを+1し、`state_lock`の下で全coderのカウンタを確認、全員が閾値到達してたらそのcoder自身がstop_flagを立てる
どちらも「立てるのは一回だけ」という保証を`state_lock`の排他で担保（既にstop_flag=1なら上書きしない、という設計——特にmonitor側のcheck_burnoutで明示的にチェックしてる）。






dongleについて
結局一人か二人なんだからヒープを持つ必要すらなくない？配列でいいじゃん。ヒープ自体を使っているのってt_dongleだけで、それ以外はいらなそう（あとヒープ自体を動かす関数）だからいらないと思うんだけど。名前上ヒープになっているだけで挙動は完全にただのt_hnode配列だね。ヒープ操作関数もヒープっていうガワだけかぶったみたいな？
dongleは隣接2人からしか要求されない。だから各dongleの待ち行列は最大2人。全体で1つのheapにまとめると、無関係な資源同士の待ち合いが発生して、本来並行実行できる部分（例えばn=4でcoder1と3が同時compile）まで不要に直列化してしまう。要求文言も"同一dongleを巡る競合"と明記してるので、per-dongle scopeが仕様に忠実かつ正しい設計


n=1の特殊ケースの場合は2個以上のドングルが存在しない→コンパイルなどの一連の動きができない→エラー（システム上クラッシュではない、意図した例外措置）措置をとっている？そうなったらデッドロック回避で二重ロックにならないよう一回だけ取得する分岐をコードないに書いているということ？その条件分岐で取得とかではなくてエラーを示す返り値（この場合は０）をreturnすればそれで良いのでは？
課題文引用：
	There is one dongle between each pair of coders... If there is only one coder, there should be only one dongle on the table.
	Reject invalid inputs such as negative numbers, non-integers, or a scheduler other than fifo or edf.

fifo/edfの切り替えについて
SCH_EDFの条件時、keyにはlast_compile_start + time_to_burnoutが書き込まれるが、なぜtrim_to_burnoutが足されている？time_to_burnoutは定数だからlast_compile_startだけで良いのでは？どうせstop_flagで判定するし、肝心のstop_flagを操作しているのはbump_compile_countくらいだね。あとis_stoppedか。

coderスレッドのライフサイクルについて
ドングルの使用が必須なのはコンパイル時のみで本当にあっているか？そのコーダーあたりのスレッドがsleepしている間も、スリープしていない他のコーダーたちは待機できる、ということね。スリープ中はlast_compile_startのカウントは動いてるよね（gettimeofday準拠だから）、ビジーループを防止するのが主な目的か。あとstop_flagはdo_compile時に判定が起きているが、コンパイルした後の一連の流れ（refactorまで）は止まって、compileで終わっちゃわない？これは課題が求めていること？

終了判定について
stop_flagはsimに１つ存在するのみだが、シミュレーターの関係で１つのコーダーがn_compiles_requiredを満たした時点でそのスレッドを個別に終了とかはしない？n_compiles_requiredが各コーダーそれぞれに適応されるのはまあわかるが（compile_count参照）、終了コード自体は全体で適用されるのはあまり良い実装とは言えないのでは？