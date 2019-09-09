# Create Device with Brandnew
Windows 8에서 처음 등장한 윈도우 스토어 앱은 새로운 구조의 API를 사용합니다. 이에 맞춰 HWND를 사용하지 않는 환경을 위한 DXGI 교환사슬 생성 API가 필요하게 되었습니다.

DXGI 1.2에서는 교환사슬 생성 API가 세분화되어 HWND과 Core Window의 생성 함수가 분리되었습니다. 더불어 교환사슬의 속성과 전체화면 속성이 분리되어 교환사슬의 백버퍼 크기를 건드릴 필요 없이 전체화면 전환도 가능합니다. 이 API를 사용하기 위해서는 Direct3D 11 초창기에 제공된 ```D3D11CreateDeviceWithSwapChain``` 함수를 사용할 수 없습니다.

Windows 10에서 개선된 DXGI 1.4에서는 교환 효과에 Flip Discard가 추가되어 창모드 및 프레임없는 전체화면 창모드에서 독점모드 전체화면과 비슷한 백버퍼 교환 속도를 낼 수 있게 되었습니다. 이 경우 백버퍼 갯수는 2개 이상이 되어야 합니다.