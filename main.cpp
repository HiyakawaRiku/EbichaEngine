#include "ConvertString.h"
#include <format>

//ファイルに書いたり読んだりするライブラリ
#include <fstream>
//時間を扱うライブラリ
#include <chrono>



#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

#include <wrl.h>


#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")



#include "EMath.h"
#include "Input.h"
#include "EWindow.h"
#include "EDirectX.h"


#include <random>
std::random_device seedGenerator;
std::mt19937 randomEngine(seedGenerator());

#include "Logger.h"
using namespace Logger;

struct VertexData {
	Vector4 position;
	Vector2 texcoord;
	Vector3 normal;
};

struct Material {
	Vector4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix4x4 uvTransform;
};

struct TransformationMatrix {
	Matrix4x4 WVP;
	Matrix4x4 World;
};

struct DirectionalLight {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

struct MaterialData {
	std::string textureFilePath;
};

struct ModelData {
	std::vector<VertexData> vertices;
	MaterialData material;
};

// チャンクヘッダ
struct ChunkHeader
{
	char id[4]; // チャンク毎のID
	int32_t size; // チャンクサイズ
};

// RIFFヘッダチャンク
struct RiffHeader
{
	ChunkHeader chunk; // "RIFF"
	char type[4]; // "WAVE"
};

// FMTチャンク
struct FormatChunk
{
	ChunkHeader chunk; // "fmt "
	WAVEFORMATEX fmt; // 波形フォーマット
};

// 音声データ
struct SoundData
{
	// 波形フォーマット
	WAVEFORMATEX wfex;
	// バッファの先頭アドレス
	BYTE* pBuffer;
	// バッファのサイズ
	unsigned int bufferSize;
};

struct Particle {
	Transform transform;
	Vector3 velocity;
	Vector4 color;
	float lifeTime;
	float currentTime;
};

struct ParticleForGPU {
	Matrix4x4 WVP;
	Matrix4x4 World;
	Vector4 color;
};


struct Emitter {
	Transform transform;
	uint32_t count;
	float frequency;//発生頻度
	float frequencyTime;//頻度用時刻
};

struct AABB {
	Vector3 min;
	Vector3 max;
};

bool IsCollision(const AABB& aabb, const Vector3& point) {
	return (point.x >= aabb.min.x && point.x <= aabb.max.x) &&
		(point.y >= aabb.min.y && point.y <= aabb.max.y) &&
		(point.z >= aabb.min.z && point.z <= aabb.max.z);
}

struct AccelerationField {
	Vector3 acceleration;
	AABB area;
};


Particle MakeNewParticle(std::mt19937& randomEngine, const Vector3& translate)
{
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
	std::uniform_real_distribution<float> distColor(0.0f, 1.0f);
	std::uniform_real_distribution<float> distTime(1.0f, 3.0f);
	Particle particle;
	Vector3 randomTranslate{ distribution(randomEngine),distribution(randomEngine),distribution(randomEngine) };
	particle.transform.scale = { 1.0f, 1.0f, 1.0f };
	particle.transform.rotate = { 0.0f, 0.0f, 0.0f };
	particle.transform.translate = translate + randomTranslate;
	particle.velocity = { distribution(randomEngine), distribution(randomEngine), distribution(randomEngine) };
	particle.color = { distColor(randomEngine), distColor(randomEngine), distColor(randomEngine),1.0f };
	particle.lifeTime = distTime(randomEngine);
	particle.currentTime = 0;
	return particle;
}

std::list<Particle> Emit(const Emitter& emitter, std::mt19937& randomEngine) {
	std::list<Particle> particles;
	for (uint32_t count = 0; count < emitter.count; ++count) {
		particles.push_back(MakeNewParticle(randomEngine, emitter.transform.translate));
	}
	return particles;
}

enum BlendMode {
	//!< ブレンドなし
	kBlendModeNone,
	//!< 通常αブレンド。デフォルト。 Src * SrcA + Dest * (1 - SrcA)
	kBlendModeNormal,
	//!< 加算。Src * SrcA + Dest * 1
	kBlendModeAdd,
	//!< 減算。Dest * 1 - Src * SrcA
	kBlendModeSubtract,
	//!< 乗算。Src * 0 + Dest * Src
	kBlendModeMultily,
	//!< スクリーン。Src * (1 - Dest) + Dest * 1
	kBlendModeScreen,
	// 利用してはいけない
	kCountOfBlendMode,
};


const float kDeltaTime = 1.0f / 60.0f;


MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename)
{
	// 1. 中で必要となる変数の宣言
	MaterialData materialData; // 構築するMaterialData
	std::string line; // ファイルから読んだ1行を格納するもの

	// 2. ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める

	// 3. 実際にファイルを読み、MaterialDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		// identifierに応じた処理
		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			// 連結してファイルパスにする
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	// 4. MaterialData を返す
	return materialData;
}

ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename)
{
	// 1. 中で必要となる変数の宣言
	ModelData modelData; // 構築するModelData
	std::vector<Vector4> positions; // 位置
	std::vector<Vector3> normals; // 法線
	std::vector<Vector2> texcoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	// 2. ファイルを開く
	std::ifstream file(directoryPath + "/" + filename); // ファイルを開く
	assert(file.is_open()); // とりあえず開けなかったら止める

	// 3. 実際にファイルを読み、ModelDataを構築していく
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む

		// identifierに応じた処理
		if (identifier == "v") {
			Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.x *= -1.0f;
			position.w = 1.0f;
			positions.push_back(position);
		}
		else if (identifier == "vt") {
			Vector2 texcoord;
			s >> texcoord.x >> texcoord.y;
			texcoord.y = 1.0f - texcoord.y;
			texcoords.push_back(texcoord);
		}
		else if (identifier == "vn") {
			Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		}
		else if (identifier == "f") {
			VertexData triangle[3];
			// 面は三角形限定。その他は未対応
			for (int32_t faceVertex = 0; faceVertex < 3; ++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexは「位置/UV/法線」で格納されているので、分解してIndexを取得する
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					std::getline(v, index, '/'); // '/'区切りでインデックスを読んでいく
					elementIndices[element] = std::stoi(index);
				}
				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				Vector4 position = positions[elementIndices[0] - 1];
				Vector2 texcoord = texcoords[elementIndices[1] - 1];
				Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}
			//頂点を逆順に登録することで、回り順を逆にする
			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);
		}
		else if (identifier == "mtllib") {
			// materialTemplateLibraryファイルの名前を取得する
			std::string materialFilename;
			s >> materialFilename;
			// 基本的にobjファイルと同一階層にmtlは存在させるので、ディレクトリ名とファイル名を渡す
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}

	// 4. ModelDataを返す
	return modelData;
}


struct D3DResourceLeakChecker {
	~D3DResourceLeakChecker() {
		// リソースリークチェック
		Microsoft::WRL::ComPtr<IDXGIDebug1> debug;
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(&debug)))) {
			debug->ReportLiveObjects(DXGI_DEBUG_ALL, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_APP, DXGI_DEBUG_RLO_ALL);
			debug->ReportLiveObjects(DXGI_DEBUG_D3D12, DXGI_DEBUG_RLO_ALL);
		}
	}
};


SoundData SoundLoadWave(const char* filename)
{
	// ファイル入力ストリームのインスタンス
	std::ifstream file;
	// .wavファイルをバイナリモードで開く
	file.open(filename, std::ios_base::binary);
	// ファイルオープン失敗を検出する
	assert(file.is_open());


	// RIFFヘッダーの読み込み
	RiffHeader riff;
	file.read((char*)&riff, sizeof(riff));
	// ファイルがRIFFかチェック
	if (strncmp(riff.chunk.id, "RIFF", 4) != 0) {
		assert(0);
	}
	// タイプがWAVEかチェック
	if (strncmp(riff.type, "WAVE", 4) != 0) {
		assert(0);
	}

	// Formatチャンクの読み込み
	FormatChunk format = {};
	// チャンクヘッダーの確認
	file.read((char*)&format, sizeof(ChunkHeader));
	if (strncmp(format.chunk.id, "fmt ", 4) != 0) {
		assert(0);
	}

	// チャンク本体の読み込み
	assert(format.chunk.size <= sizeof(format.fmt));
	file.read((char*)&format.fmt, format.chunk.size);

	// Dataチャンクの読み込み
	ChunkHeader data;
	file.read((char*)&data, sizeof(data));
	// JUNKチャンクを検出した場合
	if (strncmp(data.id, "JUNK", 4) == 0) {
		// 読み取り位置をJUNKチャンクの終わりまで進める
		file.seekg(data.size, std::ios_base::cur);
		// 再読み込み
		file.read((char*)&data, sizeof(data));
	}

	if (strncmp(data.id, "data", 4) != 0) {
		assert(0);
	}

	// Dataチャンクのデータ部（波形データ）の読み込み
	char* pBuffer = new char[data.size];
	file.read(pBuffer, data.size);

	// Waveファイルを閉じる
	file.close();

	// returnする為の音声データ
	SoundData soundData = {};

	soundData.wfex = format.fmt;
	soundData.pBuffer = reinterpret_cast<BYTE*>(pBuffer);
	soundData.bufferSize = data.size;

	return soundData;
}


// 音声データ解放
void SoundUnload(SoundData* soundData)
{
	// バッファのメモリを解放
	delete[] soundData->pBuffer;

	soundData->pBuffer = 0;
	soundData->bufferSize = 0;
	soundData->wfex = {};
}


// 音声再生
void SoundPlayWave(IXAudio2* xAudio2, const SoundData& soundData) {

	HRESULT hr;

	// 波形フォーマットを元にSourceVoiceの生成
	IXAudio2SourceVoice* pSourceVoice = nullptr;
	hr = xAudio2->CreateSourceVoice(&pSourceVoice, &soundData.wfex);
	assert(SUCCEEDED(hr));

	// 再生する波形データの設定
	XAUDIO2_BUFFER buf{};
	buf.pAudioData = soundData.pBuffer;
	buf.AudioBytes = soundData.bufferSize;
	buf.Flags = XAUDIO2_END_OF_STREAM;

	// 波形データの再生
	hr = pSourceVoice->SubmitSourceBuffer(&buf);
	hr = pSourceVoice->Start();
}


//Windowsアプリでのエントリーポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	D3DResourceLeakChecker leakCheck;

	std::unique_ptr<EWindow> eWindow = std::make_unique<EWindow>();
	eWindow->Initialize();


	std::unique_ptr<EDirectX>eDirectX = std::make_unique<EDirectX>();
	eDirectX->Initialize(eWindow.get());


	// 現在時刻を取得 (UTC時刻)
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	// ログファイルの名前コンマ何秒はいらないので、削って秒にする
	std::chrono::time_point<std::chrono::system_clock, std::chrono::seconds>
		nowSeconds = std::chrono::time_point_cast<std::chrono::seconds>(now);
	// 日本時間 (PCの設定時間) に変換
	std::chrono::zoned_time localTime{ std::chrono::current_zone(), nowSeconds };
	// formatを使って年月日_時分秒の文字列に変換
	std::string dateString = std::format("{:%Y%m%d_%H%M%S}", localTime);
	// 時刻を使ってファイル名を決定
	std::string logFilePath = std::string("logs/") + dateString + ".log";
	// ファイルを作って書き込み準備
	std::ofstream logStream(logFilePath);



	std::unique_ptr<Input> input = std::make_unique<Input>();
	input->Initialize(eWindow.get());




	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC object3dDescriptionRootSignature{};
	object3dDescriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_ROOT_SIGNATURE_DESC particleDescriptionRootSignature{};
	particleDescriptionRootSignature.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRange[0].NumDescriptors = 1;  // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;  // Offsetを自動計算

	D3D12_DESCRIPTOR_RANGE descriptorRangeForInstancing[1] = {};
	descriptorRangeForInstancing[0].BaseShaderRegister = 0;  // 0から始まる
	descriptorRangeForInstancing[0].NumDescriptors = 1;  // 数は1つ
	descriptorRangeForInstancing[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;  // SRVを使う
	descriptorRangeForInstancing[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;  // Offsetを自動計算

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER object3dRootParameters[4] = {};

	object3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;   // PixelShaderで使う
	object3dRootParameters[0].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	object3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;  // VertexShaderで使う
	object3dRootParameters[1].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	object3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	object3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	object3dRootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	object3dRootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	object3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	object3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	object3dRootParameters[3].Descriptor.ShaderRegister = 1;   // レジスタ番号1を使う
	object3dDescriptionRootSignature.pParameters = object3dRootParameters;   // ルートパラメータ配列へのポインタ
	object3dDescriptionRootSignature.NumParameters = _countof(object3dRootParameters);   // 配列の長さ

	// RootParameter作成。PixelShaderのMaterialとVertexShaderのTransform
	D3D12_ROOT_PARAMETER particleRootParameters[4] = {};

	particleRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	particleRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;   // PixelShaderで使う
	particleRootParameters[0].Descriptor.ShaderRegister = 0;   // レジスタ番号0を使う
	particleRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	particleRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX; // VertexShaderを使う
	particleRootParameters[1].DescriptorTable.pDescriptorRanges = descriptorRangeForInstancing; // Tableの中身の配列を指定
	particleRootParameters[1].DescriptorTable.NumDescriptorRanges = _countof(descriptorRangeForInstancing); // Tableで利用する数
	particleRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	particleRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	particleRootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	particleRootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	particleRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;     // CBVを使う
	particleRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;  // PixelShaderで使う
	particleRootParameters[3].Descriptor.ShaderRegister = 1;   // レジスタ番号1を使う
	particleDescriptionRootSignature.pParameters = particleRootParameters;   // ルートパラメータ配列へのポインタ
	particleDescriptionRootSignature.NumParameters = _countof(particleRootParameters);   // 配列の長さ


	D3D12_STATIC_SAMPLER_DESC object3dStaticSamplers[1] = {};
	object3dStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	object3dStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	object3dStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	object3dStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	object3dStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	object3dStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	object3dStaticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	object3dStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	object3dDescriptionRootSignature.pStaticSamplers = object3dStaticSamplers;
	object3dDescriptionRootSignature.NumStaticSamplers = _countof(object3dStaticSamplers);

	D3D12_STATIC_SAMPLER_DESC particleStaticSamplers[1] = {};
	particleStaticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	particleStaticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP; // 0~1の範囲外をリピート
	particleStaticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	particleStaticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER; // 比較しない
	particleStaticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX; // ありったけのMipmapを使う
	particleStaticSamplers[0].ShaderRegister = 0; // レジスタ番号0を使う
	particleStaticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderを使う
	particleDescriptionRootSignature.pStaticSamplers = particleStaticSamplers;
	particleDescriptionRootSignature.NumStaticSamplers = _countof(particleStaticSamplers);


	// シリアライズしてバイナリにする
	ID3DBlob* object3dSignatureBlob = nullptr;
	ID3DBlob* object3dErrorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&object3dDescriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &object3dSignatureBlob, &object3dErrorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(object3dErrorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> object3dRootSignature = nullptr;
	hr = eDirectX->GetDevice()->CreateRootSignature(0,
		object3dSignatureBlob->GetBufferPointer(), object3dSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&object3dRootSignature));
	assert(SUCCEEDED(hr));

	// シリアライズしてバイナリにする
	ID3DBlob* particleSignatureBlob = nullptr;
	ID3DBlob* particleErrorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&particleDescriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &particleSignatureBlob, &particleErrorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(particleErrorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> particleRootSignature = nullptr;
	hr = eDirectX->GetDevice()->CreateRootSignature(0,
		particleSignatureBlob->GetBufferPointer(), particleSignatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&particleRootSignature));
	assert(SUCCEEDED(hr));


	// InputLayout
	D3D12_INPUT_ELEMENT_DESC object3dInputElementDescs[3] = {};
	object3dInputElementDescs[0].SemanticName = "POSITION";
	object3dInputElementDescs[0].SemanticIndex = 0;
	object3dInputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	object3dInputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	object3dInputElementDescs[1].SemanticName = "TEXCOORD";
	object3dInputElementDescs[1].SemanticIndex = 0;
	object3dInputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	object3dInputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	object3dInputElementDescs[2].SemanticName = "NORMAL";
	object3dInputElementDescs[2].SemanticIndex = 0;
	object3dInputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	object3dInputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC object3dInputLayoutDesc{};
	object3dInputLayoutDesc.pInputElementDescs = object3dInputElementDescs;
	object3dInputLayoutDesc.NumElements = _countof(object3dInputElementDescs);

	// InputLayout
	D3D12_INPUT_ELEMENT_DESC particleInputElementDescs[3] = {};
	particleInputElementDescs[0].SemanticName = "POSITION";
	particleInputElementDescs[0].SemanticIndex = 0;
	particleInputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	particleInputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	particleInputElementDescs[1].SemanticName = "TEXCOORD";
	particleInputElementDescs[1].SemanticIndex = 0;
	particleInputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	particleInputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	particleInputElementDescs[2].SemanticName = "COLOR";
	particleInputElementDescs[2].SemanticIndex = 0;
	particleInputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	particleInputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC particleInputLayoutDesc{};
	particleInputLayoutDesc.pInputElementDescs = particleInputElementDescs;
	particleInputLayoutDesc.NumElements = _countof(particleInputElementDescs);


	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc[6]{};

	blendDesc[0].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	blendDesc[1].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[1].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[1].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[1].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[1].RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendDesc[1].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[1].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[1].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[2].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[2].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[2].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[2].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[2].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[2].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[2].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[2].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[3].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[3].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[3].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[3].RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
	blendDesc[3].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[3].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[3].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[3].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[4].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[4].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[4].RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
	blendDesc[4].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[4].RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
	blendDesc[4].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[4].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[4].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

	blendDesc[5].RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
	blendDesc[5].RenderTarget[0].BlendEnable = TRUE;
	blendDesc[5].RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendDesc[5].RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc[5].RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
	blendDesc[5].RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc[5].RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc[5].RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;


	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面（時計回り）を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;


	// Shaderをコンパイルする
	IDxcBlob* object3dVertexShaderBlob = eDirectX->CompileShader(L"Object3D.VS.hlsl",
		L"vs_6_0");
	assert(object3dVertexShaderBlob != nullptr);

	IDxcBlob* object3dPixelShaderBlob = eDirectX->CompileShader(L"Object3D.PS.hlsl", L"ps_6_0");
	assert(object3dPixelShaderBlob != nullptr);

	// Shaderをコンパイルする
	IDxcBlob* particleVertexShaderBlob = eDirectX->CompileShader(L"Particle.VS.hlsl",
		L"vs_6_0");
	assert(particleVertexShaderBlob != nullptr);

	IDxcBlob* particlePixelShaderBlob = eDirectX->CompileShader(L"Particle.PS.hlsl", L"ps_6_0");
	assert(particlePixelShaderBlob != nullptr);


	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC object3dDepthStencilDesc{};
	// Depthの機能を有効化する
	object3dDepthStencilDesc.DepthEnable = true;
	// 書き込みします
	object3dDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLessEqual。つまり、近ければ描画される
	object3dDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC particleDepthStencilDesc{};
	// Depthの機能を有効化する
	particleDepthStencilDesc.DepthEnable = true;
	// 書き込みします
	particleDepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	// 比較関数はLessEqual。つまり、近ければ描画される
	particleDepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;


	D3D12_GRAPHICS_PIPELINE_STATE_DESC object3dPipelineStateDesc{};
	object3dPipelineStateDesc.pRootSignature = object3dRootSignature.Get(); // RootSignature
	object3dPipelineStateDesc.InputLayout = object3dInputLayoutDesc; // InputLayout
	object3dPipelineStateDesc.VS = { object3dVertexShaderBlob->GetBufferPointer(),
		object3dVertexShaderBlob->GetBufferSize() }; // VertexShader
	object3dPipelineStateDesc.PS = { object3dPixelShaderBlob->GetBufferPointer(),
		object3dPixelShaderBlob->GetBufferSize() }; // Pixel Shader
	object3dPipelineStateDesc.BlendState = blendDesc[1]; // BlendState
	object3dPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	object3dPipelineStateDesc.NumRenderTargets = 1;
	object3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	object3dPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	object3dPipelineStateDesc.SampleDesc.Count = 1;
	object3dPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	object3dPipelineStateDesc.DepthStencilState = object3dDepthStencilDesc;
	object3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> object3dPipelineState = nullptr;
	hr = eDirectX->GetDevice()->CreateGraphicsPipelineState(&object3dPipelineStateDesc,
		IID_PPV_ARGS(&object3dPipelineState));
	assert(SUCCEEDED(hr));


	D3D12_GRAPHICS_PIPELINE_STATE_DESC particlePipelineStateDesc{};
	particlePipelineStateDesc.pRootSignature = particleRootSignature.Get(); // RootSignature
	particlePipelineStateDesc.InputLayout = particleInputLayoutDesc; // InputLayout
	particlePipelineStateDesc.VS = { particleVertexShaderBlob->GetBufferPointer(),
		particleVertexShaderBlob->GetBufferSize() }; // VertexShader
	particlePipelineStateDesc.PS = { particlePixelShaderBlob->GetBufferPointer(),
		particlePixelShaderBlob->GetBufferSize() }; // Pixel Shader
	particlePipelineStateDesc.BlendState = blendDesc[2]; // BlendState
	particlePipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	particlePipelineStateDesc.NumRenderTargets = 1;
	particlePipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ（形状）のタイプ。三角形
	particlePipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定（気にしなくて良い）
	particlePipelineStateDesc.SampleDesc.Count = 1;
	particlePipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	particlePipelineStateDesc.DepthStencilState = particleDepthStencilDesc;
	particlePipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> particlePipelineState = nullptr;
	hr = eDirectX->GetDevice()->CreateGraphicsPipelineState(&particlePipelineStateDesc,
		IID_PPV_ARGS(&particlePipelineState));
	assert(SUCCEEDED(hr));



	//const float pi = std::numbers::pi_v<float>;

	//const uint32_t kSubdivision = 16;
	//const size_t kTotalVertices = kSubdivision * kSubdivision * 6;


	// モデル読み込み
	ModelData modelData = LoadObjFile("resources", "plane.obj");
	// 頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource = eDirectX->CreateBufferResource(sizeof(VertexData) * modelData.vertices.size());
	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView{};
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress(); // リソースの先頭のアドレスから使う
	vertexBufferView.SizeInBytes = UINT(sizeof(VertexData) * modelData.vertices.size()); // 使用するリソースのサイズは頂点のサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData); // 1頂点あたりのサイズ

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData)); // 書き込むためのアドレスを取得
	std::memcpy(vertexData, modelData.vertices.data(), sizeof(VertexData) * modelData.vertices.size()); // 頂点データをリソースにコピー


	//// 左下
	//vertexData[0].position = { -0.5f, -0.5f, 0.0f, 1.0f };
	//vertexData[0].texcoord = { 0.0f, 1.0f };
	//// 上
	//vertexData[1].position = { 0.0f, 0.5f, 0.0f, 1.0f };
	//vertexData[1].texcoord = { 0.5f, 0.0f };
	//// 右下
	//vertexData[2].position = { 0.5f, -0.5f, 0.0f, 1.0f };
	//vertexData[2].texcoord = { 1.0f, 1.0f };
	//// 左下2
	//vertexData[3].position = { -0.5f, -0.5f, 0.5f, 1.0f };
	//vertexData[3].texcoord = { 0.0f, 1.0f };
	//// 上2
	//vertexData[4].position = { 0.0f, 0.0f, 0.0f, 1.0f };
	//vertexData[4].texcoord = { 0.5f, 0.0f };
	//// 右下2
	//vertexData[5].position = { 0.5f, -0.5f, -0.5f, 1.0f };
	//vertexData[5].texcoord = { 1.0f, 1.0f };




	//// 経度分割1つ分の角度 φ_d
	//const float kLonEvery = pi * 2.0f / float(kSubdivision);
	//// 緯度分割1つ分の角度 θ_d
	//const float kLatEvery = pi / float(kSubdivision);
	//// 緯度の方向に分割
	//for (uint32_t latIndex = 0; latIndex < kSubdivision; ++latIndex) {
	//	float lat = -pi / 2.0f + kLatEvery * latIndex;// θ

	//	float v0 = 1.0f - float(latIndex) / float(kSubdivision);       // 下側の緯度
	//	float v1 = 1.0f - float(latIndex + 1) / float(kSubdivision);   // 上側の緯度

	//	// 経度の方向に分割しながら線を描く
	//	for (uint32_t lonIndex = 0; lonIndex < kSubdivision; ++lonIndex) {
	//		uint32_t start = (latIndex * kSubdivision + lonIndex) * 6;
	//		float lon = lonIndex * kLonEvery;// φ

	//		float u0 = float(lonIndex) / float(kSubdivision);          // 左側の経度
	//		float u1 = float(lonIndex + 1) / float(kSubdivision);      // 右側の経度

	//		// 頂点にデータを入力する。基準点a
	//		vertexData[start + 0].position.x = cos(lat) * cos(lon);
	//		vertexData[start + 0].position.y = sin(lat);
	//		vertexData[start + 0].position.z = cos(lat) * sin(lon);
	//		vertexData[start + 0].position.w = 1.0f;
	//		vertexData[start + 0].texcoord = { u0, v0 };
	//		vertexData[start + 0].normal.x = vertexData[start + 0].position.x;
	//		vertexData[start + 0].normal.y = vertexData[start + 0].position.y;
	//		vertexData[start + 0].normal.z = vertexData[start + 0].position.z;
	//		// 頂点にデータを入力する。基準点b
	//		vertexData[start + 1].position.x = cos(lat + kLatEvery) * cos(lon);
	//		vertexData[start + 1].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 1].position.z = cos(lat + kLatEvery) * sin(lon);
	//		vertexData[start + 1].position.w = 1.0f;
	//		vertexData[start + 1].texcoord = { u0, v1 };
	//		vertexData[start + 1].normal.x = vertexData[start + 1].position.x;
	//		vertexData[start + 1].normal.y = vertexData[start + 1].position.y;
	//		vertexData[start + 1].normal.z = vertexData[start + 1].position.z;
	//		// 頂点にデータを入力する。基準点c
	//		vertexData[start + 2].position.x = cos(lat) * cos(lon + kLonEvery);
	//		vertexData[start + 2].position.y = sin(lat);
	//		vertexData[start + 2].position.z = cos(lat) * sin(lon + kLonEvery);
	//		vertexData[start + 2].position.w = 1.0f;
	//		vertexData[start + 2].texcoord = { u1, v0 };
	//		vertexData[start + 2].normal.x = vertexData[start + 2].position.x;
	//		vertexData[start + 2].normal.y = vertexData[start + 2].position.y;
	//		vertexData[start + 2].normal.z = vertexData[start + 2].position.z;
	//		// 頂点にデータを入力する。基準点b
	//		vertexData[start + 3].position.x = cos(lat + kLatEvery) * cos(lon);
	//		vertexData[start + 3].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 3].position.z = cos(lat + kLatEvery) * sin(lon);
	//		vertexData[start + 3].position.w = 1.0f;
	//		vertexData[start + 3].texcoord = { u0, v1 };
	//		vertexData[start + 3].normal.x = vertexData[start + 3].position.x;
	//		vertexData[start + 3].normal.y = vertexData[start + 3].position.y;
	//		vertexData[start + 3].normal.z = vertexData[start + 3].position.z;
	//		// 頂点にデータを入力する。基準点d
	//		vertexData[start + 4].position.x = cos(lat + kLatEvery) * cos(lon + kLonEvery);
	//		vertexData[start + 4].position.y = sin(lat + kLatEvery);
	//		vertexData[start + 4].position.z = cos(lat + kLatEvery) * sin(lon + kLonEvery);
	//		vertexData[start + 4].position.w = 1.0f;
	//		vertexData[start + 4].texcoord = { u1, v1 };
	//		vertexData[start + 4].normal.x = vertexData[start + 4].position.x;
	//		vertexData[start + 4].normal.y = vertexData[start + 4].position.y;
	//		vertexData[start + 4].normal.z = vertexData[start + 4].position.z;
	//		// 頂点にデータを入力する。基準点c
	//		vertexData[start + 5].position.x = cos(lat) * cos(lon + kLonEvery);
	//		vertexData[start + 5].position.y = sin(lat);
	//		vertexData[start + 5].position.z = cos(lat) * sin(lon + kLonEvery);
	//		vertexData[start + 5].position.w = 1.0f;
	//		vertexData[start + 5].texcoord = { u1, v0 };
	//		vertexData[start + 5].normal.x = vertexData[start + 5].position.x;
	//		vertexData[start + 5].normal.y = vertexData[start + 5].position.y;
	//		vertexData[start + 5].normal.z = vertexData[start + 5].position.z;
	//	}
	//}


	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource = eDirectX->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialData = nullptr;
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = true;
	materialData->uvTransform = MakeIdentity4x4();


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> wvpResource = eDirectX->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* wvpData = nullptr;
	// 書き込むためのアドレスを取得
	wvpResource->Map(0, nullptr, reinterpret_cast<void**>(&wvpData));
	// 単位行列を書きこんでおく
	wvpData->WVP = MakeIdentity4x4();
	wvpData->World = MakeIdentity4x4();

	const uint32_t kNumMaxInstance = 100;// インスタンス数
	// Instancing用のTransformationMatrixリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource = eDirectX->CreateBufferResource(sizeof(ParticleForGPU) * kNumMaxInstance);
	// データを書き込む
	ParticleForGPU* instancingData = nullptr;
	// 書き込むためのアドレスを取得
	instancingResource->Map(0, nullptr, reinterpret_cast<void**>(&instancingData));
	// 単位行列を書きこんでおく
	for (uint32_t index = 0; index < kNumMaxInstance; ++index) {
		instancingData[index].WVP = MakeIdentity4x4();
		instancingData[index].World = MakeIdentity4x4();
		instancingData[index].color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	}


	// WVP用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = eDirectX->CreateBufferResource(sizeof(DirectionalLight));
	// データを書き込む
	DirectionalLight* directionalLightData = nullptr;
	// 書き込むためのアドレスを取得
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	// 単位行列を書きこんでおく
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;



	//Transform関数を作る
	Transform transform{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
	Transform cameraTransform{ { 1.0f,1.0f,1.0f },{ std::numbers::pi_v<float> / 3.0f,std::numbers::pi_v<float>,0.0f },{ 0.0f,23.0f,10.0f } };



	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = LoadTexture("resources/uvChecker.png");
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource = eDirectX->CreateTextureResource(metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = eDirectX->UploadTextureData(textureResource, mipImages);


	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages2 = LoadTexture(modelData.material.textureFilePath);
	const DirectX::TexMetadata& metadata2 = mipImages2.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource2 = eDirectX->CreateTextureResource(metadata2);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource2 = eDirectX->UploadTextureData(textureResource2, mipImages2);

	// 2枚目のTextureを読んで転送する
	DirectX::ScratchImage mipImages3 = LoadTexture("resources/circle.png");
	const DirectX::TexMetadata& metadata3 = mipImages3.GetMetadata();
	Microsoft::WRL::ComPtr<ID3D12Resource> textureResource3 = eDirectX->CreateTextureResource(metadata3);
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource3 = eDirectX->UploadTextureData(textureResource3, mipImages3);

	//
	//// コマンドリストの内容を確定させる。すべてのコマンドを積んでからCloseすること
	//hr = eDirectX->GetCommandList()->Close();
	//assert(SUCCEEDED(hr));


	//// GPUにコマンドリストの実行を行わせる
	//Microsoft::WRL::ComPtr<ID3D12CommandList> commandLists[] = { eDirectX->GetCommandList() };
	//commandQueue->ExecuteCommandLists(1, commandLists->GetAddressOf());


	//// Fenceの値を更新
	//fenceValue++;
	//// GPUがここまでたどり着いたときに、Fenceの値を指定した値に代入するようにSignalを送る
	//commandQueue->Signal(fence.Get(), fenceValue);


	//// Fenceの値が指定したSignal値にたどり着いているか確認する
	//// GetCompletedValueの初期値はFence作成時に渡した初期値
	//if (fence->GetCompletedValue() < fenceValue)
	//{
	//	// 指定したSignalにたどり着いていないので、たどり着くまで待つようにイベントを設定する
	//	fence->SetEventOnCompletion(fenceValue, fenceEvent);
	//	// イベント待つ
	//	WaitForSingleObject(fenceEvent, INFINITE);
	//}


	//// 次のフレーム用のコマンドリストを準備
	//hr = commandAllocator->Reset();
	//assert(SUCCEEDED(hr));
	//hr = commandList->Reset(commandAllocator.Get(), nullptr);
	//assert(SUCCEEDED(hr));


	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc2{};
	srvDesc2.Format = metadata2.format;
	srvDesc2.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc2.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc2.Texture2D.MipLevels = UINT(metadata2.mipLevels);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc3{};
	srvDesc3.Format = metadata3.format;
	srvDesc3.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc3.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc3.Texture2D.MipLevels = UINT(metadata3.mipLevels);

	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = kNumMaxInstance;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(ParticleForGPU);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(3);
	D3D12_GPU_DESCRIPTOR_HANDLE instancingSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(3);
	eDirectX->GetDevice()->CreateShaderResourceView(instancingResource.Get(), &instancingSrvDesc, instancingSrvHandleCPU);


	// SRVを作成するDescriptorHeapの場所を決める
	// 先頭はImGuiが使っているのでその次を使う
	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU = eDirectX->GetSRVCPUDescriptorHandle(1);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU = eDirectX->GetSRVGPUDescriptorHandle(1);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource.Get(), &srvDesc, textureSrvHandleCPU);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU2 = eDirectX->GetSRVCPUDescriptorHandle(2);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU2 = eDirectX->GetSRVGPUDescriptorHandle(2);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource2.Get(), &srvDesc2, textureSrvHandleCPU2);


	D3D12_CPU_DESCRIPTOR_HANDLE textureSrvHandleCPU3 = eDirectX->GetSRVCPUDescriptorHandle(4);
	D3D12_GPU_DESCRIPTOR_HANDLE textureSrvHandleGPU3 = eDirectX->GetSRVGPUDescriptorHandle(4);

	// SRVの生成
	eDirectX->GetDevice()->CreateShaderResourceView(textureResource3.Get(), &srvDesc3, textureSrvHandleCPU3);



	// Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResourceSprite = eDirectX->CreateBufferResource(sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する
	D3D12_VERTEX_BUFFER_VIEW vertexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	vertexBufferViewSprite.BufferLocation = vertexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferViewSprite.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferViewSprite.StrideInBytes = sizeof(VertexData);

	VertexData* vertexDataSprite = nullptr;
	vertexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&vertexDataSprite));

	// 1枚目の三角形
	vertexDataSprite[0].position = { 0.0f, 360.0f, 0.0f, 1.0f }; // 左下
	vertexDataSprite[0].texcoord = { 0.0f, 1.0f };
	vertexDataSprite[0].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[1].position = { 0.0f, 0.0f, 0.0f, 1.0f }; // 左上
	vertexDataSprite[1].texcoord = { 0.0f, 0.0f };
	vertexDataSprite[1].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[2].position = { 640.0f, 360.0f, 0.0f, 1.0f }; // 右下
	vertexDataSprite[2].texcoord = { 1.0f, 1.0f };
	vertexDataSprite[2].normal = { 0.0f,0.0f,-1.0f };
	vertexDataSprite[3].position = { 640.0f, 0.0f, 0.0f, 1.0f }; // 右上
	vertexDataSprite[3].texcoord = { 1.0f, 0.0f };
	vertexDataSprite[3].normal = { 0.0f,0.0f,-1.0f };


	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResourceSprite = eDirectX->CreateBufferResource(sizeof(Material));
	// マテリアルにデータを書き込む
	Material* materialDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	materialResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&materialDataSprite));
	// 今回は赤を書き込んでみる
	materialDataSprite->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialDataSprite->enableLighting = false;
	materialDataSprite->uvTransform = MakeIdentity4x4();


	// Sprite用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResourceSprite = eDirectX->CreateBufferResource(sizeof(TransformationMatrix));
	// データを書き込む
	TransformationMatrix* transformationMatrixDataSprite = nullptr;
	// 書き込むためのアドレスを取得
	transformationMatrixResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixDataSprite));
	// 単位行列を書きこんでおく
	transformationMatrixDataSprite->WVP = MakeIdentity4x4();
	transformationMatrixDataSprite->World = MakeIdentity4x4();


	// Sprite用の頂点リソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> indexResourceSprite = eDirectX->CreateBufferResource(sizeof(VertexData) * 6);

	D3D12_INDEX_BUFFER_VIEW indexBufferViewSprite{};
	// リソースの先頭のアドレスから使う
	indexBufferViewSprite.BufferLocation = indexResourceSprite->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferViewSprite.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferViewSprite.Format = DXGI_FORMAT_R32_UINT;


	//インデックスリソースにデータを書き込む
	uint32_t* indexDataSprite = nullptr;
	indexResourceSprite->Map(0, nullptr, reinterpret_cast<void**>(&indexDataSprite));
	indexDataSprite[0] = 0;	indexDataSprite[1] = 1;	indexDataSprite[2] = 2;
	indexDataSprite[3] = 1;	indexDataSprite[4] = 3;	indexDataSprite[5] = 2;


	Transform transformSprite{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
	Transform uvTransformSprite{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };

	std::list<Particle> particles;
	Emitter emitter{};
	emitter.transform.translate = { 0.0f,0.0f,0.0f };
	emitter.transform.rotate = { 0.0f,0.0f,0.0f };
	emitter.transform.scale = { 1.0f,1.0f,1.0f };
	emitter.count = 3;
	emitter.frequency = 0.5f;
	emitter.frequencyTime = 0.0f;

	AccelerationField accelerationField;
	accelerationField.acceleration = { 15.0f,0.0f,0.0f };
	accelerationField.area.min = { -1.0f,-1.0f,-1.0f };
	accelerationField.area.max = { 1.0f,1.0f,1.0f };


	bool useMonsterBall = true;


	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice;

	//XAudioエンジンのインスタンスを生成
	hr = XAudio2Create(&xAudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	//マスターボイスを生成
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	//音声読み込み
	SoundData soundData1 = SoundLoadWave("Resources/fanfare.wav");

	//音声再生
	SoundPlayWave(xAudio2.Get(), soundData1);


	// ウィンドウの×ボタンが押されるまでループ
	while (true) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (eWindow->ProcessMessage()) {
			break;
		}


		input->Update();

		//数字の0キーが押されていたら
		if (input->TriggerKey(DIK_0)) {
			OutputDebugStringA("Hit 0\n");//出力ウィンドウに「Hit 0」と表示
		}

		for (std::list<Particle>::iterator particleIterator = particles.begin(); particleIterator != particles.end(); ++particleIterator) {
			(*particleIterator).transform.translate.x += (*particleIterator).velocity.x * kDeltaTime;
			(*particleIterator).transform.translate.y += (*particleIterator).velocity.y * kDeltaTime;
			(*particleIterator).transform.translate.z += (*particleIterator).velocity.z * kDeltaTime;
		}


#ifdef USE_IMGUI
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//開発用UIの処理。実際に開発用のUIを出す場合はここをゲーム所有の処理に置き換える
		ImGui::ShowDemoWindow();

		ImGui::Checkbox("useMonsterBall", &useMonsterBall);

		ImGui::DragFloat2("UVTranslate", &uvTransformSprite.translate.x, 0.01f, -10.0f, 10.0f);
		ImGui::DragFloat2("UVScale", &uvTransformSprite.scale.x, 0.01f, -10.0f, 10.0f);
		ImGui::SliderAngle("UVRotate", &uvTransformSprite.rotate.z);

		ImGui::DragFloat("intensity", &directionalLightData->intensity, 0.01f, 0.0f, 10.0f);

		if (ImGui::Button("Add Particle")) {
			particles.splice(particles.end(), Emit(emitter, randomEngine));
		}

		ImGui::DragFloat3("EmitterTranslate", &emitter.transform.translate.x, 0.01f, -100.0f, 100.0f);

		//ImGuiの内部コマンドを生成する
		ImGui::Render();
#endif


		Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
		Matrix4x4 backToFrontMatrix = MakeRotateYMatrix(std::numbers::pi_v<float>);
		Matrix4x4 billboardMatrix = Multiply(backToFrontMatrix, cameraMatrix);
		billboardMatrix.m[3][0] = 0.0f;
		billboardMatrix.m[3][1] = 0.0f;
		billboardMatrix.m[3][2] = 0.0f;


		//Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(EWindow::kClientWidth) / float(EWindow::kClientHeight), 0.1f, 100.0f);
		//Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
		//wvpData->World = worldViewProjectionMatrix;
		//wvpData->WVP = worldViewProjectionMatrix;

		emitter.frequencyTime += kDeltaTime; // 時刻を進める
		if (emitter.frequency <= emitter.frequencyTime) { // 頻度より大きいなら発生
			particles.splice(particles.end(), Emit(emitter, randomEngine)); // 発生処理
			emitter.frequencyTime -= emitter.frequency; // 余計に過ぎた時間も加味して頻度計算する
		}

		uint32_t numInstance = 0;
		for (std::list<Particle>::iterator particleIterator = particles.begin(); particleIterator != particles.end();) {
			if (numInstance < kNumMaxInstance) {
				if ((*particleIterator).lifeTime <= (*particleIterator).currentTime) {
					particleIterator = particles.erase(particleIterator);
					continue;
				}

				Matrix4x4 scaleMatrix = MakeScaleMatrix((*particleIterator).transform.scale);
				Matrix4x4 translateMatrix = MakeTranslateMatrix((*particleIterator).transform.translate);

				Matrix4x4 worldMatrix = Multiply(scaleMatrix, Multiply(billboardMatrix, translateMatrix));
				Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));

				if (IsCollision(accelerationField.area, (*particleIterator).transform.translate)) {
					(*particleIterator).velocity.x += accelerationField.acceleration.x * kDeltaTime;
					(*particleIterator).velocity.y += accelerationField.acceleration.y * kDeltaTime;
					(*particleIterator).velocity.z += accelerationField.acceleration.z * kDeltaTime;
				}

				(*particleIterator).transform.translate.x += (*particleIterator).velocity.x * kDeltaTime;
				(*particleIterator).transform.translate.y += (*particleIterator).velocity.y * kDeltaTime;
				(*particleIterator).transform.translate.z += (*particleIterator).velocity.z * kDeltaTime;
				(*particleIterator).currentTime += kDeltaTime;
				instancingData[numInstance].WVP = worldViewProjectionMatrix;// 10 <= numInstance || numInstance < 0はバッファオーバーラン
				instancingData[numInstance].World = worldMatrix;
				instancingData[numInstance].color = (*particleIterator).color;
				float alpha = 1.0f - ((*particleIterator).currentTime / (*particleIterator).lifeTime);
				instancingData[numInstance].color.w = alpha;
				++numInstance;
			}
			++particleIterator;
		}

		// Sprite用のWorldViewProjectionMatrixを作る
		Matrix4x4 worldMatrixSprite = MakeAffineMatrix(transformSprite.scale, transformSprite.rotate, transformSprite.translate);
		Matrix4x4 viewMatrixSprite = MakeIdentity4x4();
		Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(EWindow::kClientWidth), float(EWindow::kClientHeight), 0.0f, 100.0f);
		Matrix4x4 worldViewProjectionMatrixSprite = Multiply(worldMatrixSprite, Multiply(viewMatrixSprite, projectionMatrixSprite));
		transformationMatrixDataSprite->World = worldViewProjectionMatrixSprite;
		transformationMatrixDataSprite->WVP = worldViewProjectionMatrixSprite;

		Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransformSprite.scale);
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransformSprite.rotate.z));
		uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransformSprite.translate));
		materialDataSprite->uvTransform = uvTransformMatrix;

		eDirectX->PreDraw();

		//// 描画用のDescriptorHeapの設定
		//Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeaps[] = { srvDescriptorHeap.Get() };
		//commandList->SetDescriptorHeaps(1, descriptorHeaps->GetAddressOf());
		//commandList->RSSetViewports(1, &viewport); // Viewportを設定
		//commandList->RSSetScissorRects(1, &scissorRect); // Scirssorを設定
		//// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		//commandList->SetGraphicsRootSignature(rootSignature.Get());
		//commandList->SetPipelineState(graphicsPipelineState.Get()); // PSOを設定
		//commandList->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
		//// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		//commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		//// マテリアルCBufferの場所を設定
		//commandList->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		//// wvp用のCBufferの場所を設定
		//commandList->SetGraphicsRootConstantBufferView(1, wvpResource->GetGPUVirtualAddress());
		//// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
		//commandList->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		//// directionalLight用のCBufferの場所を設定
		//commandList->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		//// 描画！（DrawCall/ドローコール）。3頂点で1つのインスタンス。インスタンスについては今後
		//commandList->DrawInstanced(UINT(modelData.vertices.size()), 1, 0, 0);



		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		eDirectX->GetCommandList()->SetGraphicsRootSignature(particleRootSignature.Get());
		eDirectX->GetCommandList()->SetPipelineState(particlePipelineState.Get()); // PSOを設定
		eDirectX->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
		// 形状を設定。PSOに設定しているものとはまた別。同じものを設定すると考えておけば良い
		eDirectX->GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		// マテリアルCBufferの場所を設定
		eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
		// instancing用のDataを読むためにStructuredBufferのSRVを設定する
		eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(1, instancingSrvHandleGPU);
		// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
		eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(2, useMonsterBall ? textureSrvHandleGPU2 : textureSrvHandleGPU);
		// directionalLight用のCBufferの場所を設定
		eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
		// 描画！6頂点の板ポリゴンを、kNumInstance(今回は10)だけInstance描画を行う
		if (numInstance > 0) {
			eDirectX->GetCommandList()->DrawInstanced(UINT(modelData.vertices.size()), numInstance, 0, 0);
		}

		// RootSignatureを設定。PSOに設定しているけど別途設定が必要
		eDirectX->GetCommandList()->SetGraphicsRootSignature(object3dRootSignature.Get());
		eDirectX->GetCommandList()->SetPipelineState(object3dPipelineState.Get()); // PSOを設定
		// Spriteの描画。変更が必要なものだけ変更する
		eDirectX->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferViewSprite); // VBVを設定
		eDirectX->GetCommandList()->IASetIndexBuffer(&indexBufferViewSprite); // IBVを設定
		// マテリアルCBufferの場所を設定
		eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResourceSprite->GetGPUVirtualAddress());
		// TransformationMatrixCBufferの場所を設定
		eDirectX->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResourceSprite->GetGPUVirtualAddress());
		// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
		eDirectX->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
		// 描画！（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
		eDirectX->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);


		// 実際のcommandListのImGuiの描画コマンドを積む
#ifdef USE_IMGUI
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), eDirectX->GetCommandList());
#endif

		eDirectX->PostDraw();

	}

	//XAudio2解放
	xAudio2.Reset();

	//音声データ解放
	SoundUnload(&soundData1);

	// ImGuiの終了処理。詳細はさして重要ではないので解説は省略する。
	// こういうもんである。初期化と逆順に行う
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	particleSignatureBlob->Release();
	if (particleErrorBlob) {
		particleErrorBlob->Release();
	}
	object3dSignatureBlob->Release();
	if (particleErrorBlob) {
		particleErrorBlob->Release();
	}
	particlePixelShaderBlob->Release();
	particleVertexShaderBlob->Release();
	object3dPixelShaderBlob->Release();
	object3dVertexShaderBlob->Release();



	eDirectX->Finalize();
	eWindow->Finalize();

	return 0;
}