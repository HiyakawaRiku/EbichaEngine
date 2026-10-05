#include "Sprite.h"
#include "SpriteCommon.h"

void Sprite::Initialize(SpriteCommon* spriteCommon)
{
	this->spriteCommon = spriteCommon;


	// Sprite用の頂点リソースを作る
	vertexResource = CreateBufferResource(spriteCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * 4);

	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView.BufferLocation = vertexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferView.StrideInBytes = sizeof(VertexData);

	// Sprite用の頂点リソースを作る
	indexResource = CreateBufferResource(spriteCommon->GetDxCommon()->GetDevice(), sizeof(VertexData) * 6);


	// リソースの先頭のアドレスから使う
	indexBufferView.BufferLocation = indexResource->GetGPUVirtualAddress();
	// 使用するリソースのサイズはインデックス6つ分のサイズ
	indexBufferView.SizeInBytes = sizeof(uint32_t) * 6;
	// インデックスはuint32_tとする
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;


	vertexResource->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	indexResource->Map(0, nullptr, reinterpret_cast<void**>(&indexData));


	// マテリアル用のリソースを作る。今回はcolor1つ分のサイズを用意する
	materialResource = CreateBufferResource(spriteCommon->GetDxCommon()->GetDevice(), sizeof(Material));
	// マテリアルにデータを書き込む
	// 書き込むためのアドレスを取得
	materialResource->Map(0, nullptr, reinterpret_cast<void**>(&materialData));
	// 今回は赤を書き込んでみる
	materialData->color = Vector4(1.0f, 1.0f, 1.0f, 1.0f);
	materialData->enableLighting = false;
	materialData->uvTransform = MakeIdentity4x4();


	// 用のTransformationMatrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrixResource = CreateBufferResource(spriteCommon->GetDxCommon()->GetDevice(), sizeof(TransformationMatrix));
	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationMatrixResource->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData));
	// 単位行列を書きこんでおく
	transformationMatrixData->WVP = MakeIdentity4x4();
	transformationMatrixData->World = MakeIdentity4x4();


}

void Sprite::Update()
{
	float left = 0.0f - anchorPoint.x;
	float right = 1.0f - anchorPoint.x;
	float top = 0.0f - anchorPoint.y;
	float bottom = 1.0f - anchorPoint.y;

	vertexData[0].position = { left, bottom, 0.0f, 1.0f }; // 左下
	vertexData[1].position = { left, top, 0.0f, 1.0f }; // 左上
	vertexData[2].position = { right, bottom, 0.0f, 1.0f }; // 右下
	vertexData[3].position = { right, top, 0.0f, 1.0f }; // 右上


	// 1枚目の三角形
	vertexData[0].texcoord = { 0.0f, 1.0f };
	vertexData[0].normal = { 0.0f,0.0f,-1.0f };
	vertexData[1].texcoord = { 0.0f, 0.0f };
	vertexData[1].normal = { 0.0f,0.0f,-1.0f };
	vertexData[2].texcoord = { 1.0f, 1.0f };
	vertexData[2].normal = { 0.0f,0.0f,-1.0f };
	vertexData[3].texcoord = { 1.0f, 0.0f };
	vertexData[3].normal = { 0.0f,0.0f,-1.0f };


	indexData[0] = 0;	indexData[1] = 1;	indexData[2] = 2;
	indexData[3] = 1;	indexData[4] = 3;	indexData[5] = 2;

	Transform transform{ { size.x,size.y,1.0f },{ 0.0f,0.0f,rotation },{ position.x,position.y,0.0f } };
	Transform uvTransform{ { 1.0f,1.0f,1.0f },{ 0.0f,0.0f,0.0f },{ 0.0f,0.0f,0.0f } };
	// Sprite用のWorldViewProjectionMatrixを作る
	Matrix4x4 worldMatrix = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
	Matrix4x4 viewMatrix = MakeIdentity4x4();
	Matrix4x4 projectionMatrix = MakeOrthographicMatrix(0.0f, 0.0f, float(WinApp::kClientWidth), float(WinApp::kClientHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrix = Multiply(worldMatrix, Multiply(viewMatrix, projectionMatrix));
	transformationMatrixData->WVP = worldViewProjectionMatrix;
	transformationMatrixData->World = worldMatrix;

	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform.translate));
	materialData->uvTransform = uvTransformMatrix;

}

void Sprite::Draw()
{

	// Spriteの描画。変更が必要なものだけ変更する
	spriteCommon->GetDxCommon()->GetCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView); // VBVを設定
	spriteCommon->GetDxCommon()->GetCommandList()->IASetIndexBuffer(&indexBufferView); // IBVを設定
	// マテリアルCBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialResource->GetGPUVirtualAddress());
	// TransformationMatrixCBufferの場所を設定
	spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootConstantBufferView(1, transformationMatrixResource->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定。2はrootParameters[2]である。
	//spriteCommon->GetDxCommon()->GetCommandList()->SetGraphicsRootDescriptorTable(2, textureSrvHandleGPU);
	// 描画！（DrawCall/ドローコール）6個のインデックスを使用し1つのインスタンスを描画。その他は当面0で良い
	spriteCommon->GetDxCommon()->GetCommandList()->DrawIndexedInstanced(6, 1, 0, 0, 0);

}
