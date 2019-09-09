# Create Device with Legacy API
DirectX Graphics Infrastructure(DXGI)의 레거시 교환사슬 생성 API는 Windows Vista에서 처음 등장했습니다. 이 당시의 창 시스템은 HWND(창 핸들)로 구분되는 레거시 API만을 지원합니다.

Windows Vista에서의 API는 Direct3D 10용으로 등장했지만 Windows 7에서는 Direct3D 11 논리 장치 객체를 생성할 때 교환사슬을 같이 초기화할 수 있는 함수를 제공합니다. 이 함수가 ```D3D11CreateDeviceAndSwapChain```입니다.