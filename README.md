# TeaFactory Simulator (C++17)

茶葉の製造工程（蒸し → 揉捻 → 乾燥）を、**離散時間ステップ**
（デフォルト: Δt=1秒）で進める **C++ シミュレーションエンジン**です。
UI ではなく、状態の数値ログ出力にフォーカスしています。

## 目的

- 製造プロセスの状態変化を、シンプルな決定論モデルで再現する
- ドメインモデル / プロセス / シミュレーション / CLI を分離した
  読みやすい構成を示す

## GUI 版（Dear ImGui ダッシュボード）

このリポジトリには、CLI 版に加えて **Dear ImGui + GLFW + OpenGL3** による
簡易ダッシュボード（`TeaFactorySimulator`）も含みます。

- 1 画面で工程と状態量（moisture/temp/aroma/color）を **ProgressBar** で表示
- **Start / Pause / Reset** ボタンで制御
- FINISHED で品質スコアを表示

## シミュレーションの考え方

`TeaLeaf` は以下の物理状態を持ちます。

- moisture: 水分率（0.0 – 1.0）
- temperature: 温度（°C）
- aroma: 香気（0 – 100）
- color: 色指標（0 – 100）

各工程は 1 ステップごとに `TeaLeaf` を少しずつ更新します。
式はポートフォリオ用途として「読みやすさ」と「妥当な挙動」を優先し、
複雑な物理モデルにはしません。

- 蒸し: 温度を目標値へ近づけ、水分を僅かに増やし、香気を増やす
- 揉捻: 冷却しつつ水分を減らし、香気と色（酸化など）を緩やかに変化
- 乾燥: 水分を指数減衰で強く減らし、過熱時は香気を劣化させる

工程の既定時間は以下です（合計 120 秒）。

- STEAMING: 30 秒
- ROLLING: 30 秒
- DRYING: 60 秒

## ビルド方法

### CMake（推奨）

```bash
cmake -S . -B build
cmake --build build -j
```

#### GUI 版について（依存取得）

GUI ターゲットは CMake の `FetchContent` を用いて、
`glfw` と `imgui` を Git から取得します。

- ネットワークが使える環境で実行してください
- 依存を取得したくない場合は `-DTEAFACTORY_BUILD_GUI=OFF` を指定します

### コンパイラ直叩き（CMake が無い場合）

```bash
mkdir -p build_manual
clang++ -std=c++17 -O2 -Wall -Wextra -Wpedantic -Isrc \
  src/cli/main.cpp \
  src/process/SteamingProcess.cpp \
  src/process/RollingProcess.cpp \
  src/process/DryingProcess.cpp \
  src/simulation/Simulator.cpp \
  -o build_manual/tea_factory_simulator
```

## 実行方法

```bash
./build/tea_factory_simulator
```

GUI 版（ダッシュボード）:

```bash
./build/TeaFactorySimulator
```

## CSV 出力

CLI 版は実行すると、カレントディレクトリに
`tea_factory_cli.csv` を生成します（1秒ごとに1行）。

次タスク（CLI引数化）で、出力先パス指定にも対応します。

出力例（毎ステップ出力）:

```
[STEAMING] t=30s moisture=0.78 temp=95.0 aroma=40.0 color=10.0
[ROLLING]  t=60s moisture=0.65 temp=70.0 aroma=55.0 color=20.0
[DRYING]   t=120s moisture=0.05 temp=60.0 aroma=50.0 color=25.0
```


