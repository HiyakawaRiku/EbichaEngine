#include "EbichaEngine.h"

// Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
    EbichaEngine engine;

    engine.Initialize();
    engine.Run();
    engine.Finalize();

    return 0;
}