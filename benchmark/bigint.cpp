#include "bigint/bigint.hpp"

#include <benchmark/benchmark.h>

using namespace BI;

static BigInt const a((111_bi).pow(1099));
static BigInt const b((99_bi).pow(981));
static BigInt const m((50_bi).pow(373));

static std::string const x_str =
    "144363981030239935818721580553560462227368131730499502371521898359277412633684443395577960999514832186263507753923"
    "493609983906964035483890882294975460710259548448366555433633087367028014595706084820411472768901973731763165453339"
    "876033501667326669405410887510209485052808390770847824507387540487510427378618598955279042695294230481690627998568"
    "796746570818674217024821555777069978466376571727879108852747913611973240354707260305377216000775217009081837347992"
    "915503915247528801208311753312031144720524502015155448876267426766514589360699172611600585224209603621168453789804"
    "390155226081244531283727055029835161913291967983864256982569176953460682610446841216844604917853498950514264863845"
    "465228098945365624067812222293253335497844920507593131056713337527752932142779684186360428505707948752180913043598"
    "092365436912688676534710211940201912278059225010916868018310253334931233726943393793404906869973721663313472993063"
    "435725358058816578416533940563615369017638294264315062127425023399160959936452236733638084858380734625750134369752"
    "008901500758702052806951011921793732119552247876229708484633033815230393213515639945810615370920346518559408969102"
    "079277832389575941651852072806898486127523582380079214080354278489219593480572949882912157365555768347162719799037"
    "880799231928392844366116694157281403827732872244255109697163065652440786153316016986302564127580861618653605431285"
    "414629537213781395850059270976981228193939937664884246758627619242815492608811845858151582968676860721192541725626"
    "135047060472658491747869932035794690897894308322146572378880853971609900593050107844721792044242057983669777526220"
    "829862348375941400357419093016751218142626659058032282076234140035227255285507180163569517530443476076986379539440"
    "446428528030586053137862726197515978504371676939996810503945518908554014898320155247973387234398763404352119158173"
    "164421093845383391939410629738506066953018036624079285430181176503915495839199241101246803698750196978209789840298"
    "123679272896286687016913315914195831889124634845301701148325796861479047574527270271771652112495180928265004950721"
    "826197180676607161987752392875213364409415844803406021556251926904463354032299761757742443222269280268892161130342"
    "2387839695866518082182507890549279006938749101522016727";

static std::string const x_str_hex =
    "0X94107FCABD2ACF0B3835DA322BA40E514135129E2889A0B5F663BD56685F7C5EE6E112E9A5F986A8949E5259FD501421B9DD4B79E9610BBE"
    "C1BD636BAA98273F5B1F50DE2CB76AA8FC14481362F75665CBA3C5ACC407C624A91F177C7CEE1A828D6BBBE02DC33FAB61ED9C09836B8F803C"
    "1E6D02C7EF4FC467B47C1258C152684FCBFA12C3AB8DB3343C7F7E2D358C4FE9E46E100DB35F54607731A03D14B3FB5837A4D6EFB01B332C9B"
    "8114C9A5303B11D80C4B8F1C2761497ADA3005A4E8AFB95F38BDF114C3B0D3BD8884805D01B896B34DF2B8963A3E6B4A629ADE8357888C4544"
    "2BCCE665DF7A5008A99163FB1AA5EBA92ED8AC3A123266691463F52E69AFE13F5DE9649B86F7E80D65F0FAB7A5F22F50525B09430A6F741508"
    "65C0D93CA77C4ED247BD385D4FCFA28354A5B659C3F6E89B601179A7CF664095DF23B012CA47DD4369DA89562988CBC20D3211EA51C1EDE5E5"
    "AC7F35CAED387A7F18981C9F9402ED6C9DBD822376A695DE5FDDE8ACE462A196C77C9F6B6434EBFE7BB9155C362E734DF5E913E8DC8DC256CD"
    "C83413634CCFA262251CE389D76F971CFF00668EA90610192B1CDD448247EADDA37CF90BFFF8DC0A8704E18C5E7BD6DB71FC8D9701EA1FC0C7"
    "F755A587763D850193B9801445BEBC8BC3BF7345D7C5A65F178A2113EC4AA9674D6EF618C37E39E79B3115AD58FA1629EE1CD082A69EA9E724"
    "6945D442BE6F25515E25F7B768333B7009590CA8A3D26375AB7671F8B89E5CDEBCC8752F14735F0A71A2942E099ADAA800C820DF9AB8D377E9"
    "38428EF4416A2061F643FB2D0E4293067B5CC4B949AB3A67E72D6F3D1AD7C6829D337D53438950CD07E873C09661CA72812F8196911AA3B1E1"
    "8A3DA5C066BA0B059413C77C0EE70DD7D7AD2F8BC0428A9D1BB02984ECFA746A06C70AADDE3CCBE286A7A3864D8D7AD86BB96861DB6EA31FAD"
    "94FB38B534830146C87664BB5CC5391F7872A5CDE8915F4E363322E28C5D84AD415B037D8BFFE679EDBEB9D347235BBF97826F5112B4211258"
    "A699A681C2864978FDF9601745AC708AAF2EDF03BFB58E9C703A54EE882F5EFF35509E0118EED3C7809829F9D2B4981BC960E4CC264D563C98"
    "A91825F869174CCB96ACCE28DB540883C102EE6EA1F16F3C32E652F66723B8AF83C0E76356282AE32FC9DCEAA61D7D74E4770B8E12556B7FE7"
    "B516C23026853246FD0664BA24F5F56593CBE4DD27C9DC8FCBD41B91A8DAFD92F772FB5EDFEBAE3D7FF62471D3A7F2C48C0C3D6EDC143A8117"
    "0A62829E179DB2AB27D9D7";

static void BM_BigInt_DefaultConstructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_DefaultConstructor);

static void BM_BigInt_IntegralConstructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c(0x58911895890ULL);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_IntegralConstructor);

static void BM_BigInt_StringConstructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c(x_str);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_StringConstructor);

static void BM_BigInt_StringConstructorHex(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c(x_str_hex);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_StringConstructorHex);

static void BM_BigInt_CopyConstructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_CopyConstructor);

static BigInt const temp(a);

static void BM_BigInt_MoveConstructor(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = std::move(temp);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_MoveConstructor);

static void BM_BigInt_abs(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a.abs();
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_abs);

static BigInt const n(235789783);

static void BM_BigInt_to_Integral(benchmark::State& state)
{
    for (auto _ : state)
    {
        int c = static_cast<int>(n);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_to_Integral);

static void BM_BigInt_to_String(benchmark::State& state)
{
    for (auto _ : state)
    {
        std::string c = std::format("{}", a);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_to_String);

static void BM_BigInt_to_HexString(benchmark::State& state)
{
    for (auto _ : state)
    {
        std::string c = std::format("{:X}", a);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_to_HexString);

static void BM_BigInt_UnaryPlus(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = +a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_UnaryPlus);

static void BM_BigInt_UnaryMinus(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = -a;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_UnaryMinus);

static void BM_BigInt_Comparison(benchmark::State& state)
{
    for (auto _ : state)
    {
        bool c = a < b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Comparison);

static void BM_BigInt_Addition(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a + b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Addition);

static void BM_BigInt_Subtraction(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a - b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Subtraction);

static void BM_BigInt_BitsShiftLeft(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a << 512357;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_BitsShiftLeft);

static void BM_BigInt_BitsShiftRight(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a >> 247;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_BitsShiftRight);

static void BM_BigInt_Multiplication(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a * b;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Multiplication);

static void BM_BigInt_Division(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a / m;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Division);

static void BM_BigInt_Modulus(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = a % m;
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Modulus);

static void BM_BigInt_Power(benchmark::State& state)
{
    for (auto _ : state)
    {
        BigInt c = m.pow(100);
        benchmark::DoNotOptimize(c);
    }
}
BENCHMARK(BM_BigInt_Power);

BENCHMARK_MAIN();
