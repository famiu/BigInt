#include "bigint/bigint.hpp"

#include <benchmark/benchmark.h>

using namespace BI;

static BigInt const
  a("120578912758912758913895789358923592358239579559015125189895678567893478963478934678934678934789275");
static BigInt const b("834787830367189376136623897598635623789567838957267892368577678087891248362792");
static BigInt const m("1234567825172589251948627619767135947892368236789");

static void BM_BigInt_DefaultConstructor(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_DefaultConstructor);

static void BM_BigInt_IntegralConstructor(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c(0x58911895890ULL);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_IntegralConstructor);

static void BM_BigInt_StringConstructor(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c("120578912758912758913895789358923592358239579559015125189895678567893478963478934678934678934789275");
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_StringConstructor);

static void BM_BigInt_CopyConstructor(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_CopyConstructor);

static BigInt const temp("120578912758912758919559015125189275");

static void BM_BigInt_MoveConstructor(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = std::move(temp);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_MoveConstructor);

static void BM_BigInt_Literal(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c =
          120578912758912758913895789358923592358239579559015125189895678567893478963478934678934678934789275_bi;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Literal);

static void BM_BigInt_abs(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a.abs();
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_abs);

static BigInt const n(235789783);

static void BM_BigInt_to_Integral(benchmark::State& state)
{
    for (auto _ : state) {
        int c = static_cast<int>(n);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_to_Integral);

static void BM_BigInt_to_String(benchmark::State& state)
{
    for (auto _ : state) {
        std::string c = std::string(a);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_to_String);

static void BM_BigInt_UnaryPlus(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = +a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_UnaryPlus);

static void BM_BigInt_UnaryMinus(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = -a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_UnaryMinus);

static void BM_BigInt_Comparison(benchmark::State& state)
{
    for (auto _ : state) {
        bool c = a < b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Comparison);

static void BM_BigInt_Addition(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a + b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Addition);

static void BM_BigInt_Subtraction(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a - b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Subtraction);

static void BM_BigInt_BitsShiftLeft(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a << 512357;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_BitsShiftLeft);

static void BM_BigInt_BitsShiftRight(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a >> 247;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_BitsShiftRight);

static void BM_BigInt_Multiplication(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a * b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Multiplication);

static void BM_BigInt_Division(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a / m;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Division);

static void BM_BigInt_Modulus(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a % m;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Modulus);

static void BM_BigInt_Power(benchmark::State& state)
{
    for (auto _ : state) {
        BigInt c = a.pow(100);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Power);

BENCHMARK_MAIN();
