#include"../../Application/main.h"
#include "KdEffekseerManager.h"


void KdEffekseerManager::Create(int w, int h)
{
	// エフェクトのレンダラーの作成
	m_efkRenderer = ::EffekseerRendererDX11::Renderer::Create(KdDirect3D::Instance().WorkDev(), KdDirect3D::Instance().WorkDevContext(), 1000);

	// エフェクトのマネージャーの作成
	m_efkManager = ::Effekseer::Manager::Create(1000);

	// 左手座標系に変換
	m_efkManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);

	// 描画用インスタンスから描画機能を設定
	m_efkManager->SetSpriteRenderer(m_efkRenderer->CreateSpriteRenderer());
	m_efkManager->SetRibbonRenderer(m_efkRenderer->CreateRibbonRenderer());
	m_efkManager->SetRingRenderer(m_efkRenderer->CreateRingRenderer());
	m_efkManager->SetTrackRenderer(m_efkRenderer->CreateTrackRenderer());
	m_efkManager->SetModelRenderer(m_efkRenderer->CreateModelRenderer());

	// 描画用インスタンスからテクスチャの読み込み機能を設定
	m_efkManager->SetTextureLoader(m_efkRenderer->CreateTextureLoader());
	m_efkManager->SetModelLoader(m_efkRenderer->CreateModelLoader());
	m_efkManager->SetMaterialLoader(m_efkRenderer->CreateMaterialLoader());
	m_efkManager->SetCurveLoader(Effekseer::MakeRefPtr<Effekseer::CurveLoader>());

	// 投影行列を設定
	m_efkRenderer->SetProjectionMatrix(
		::Effekseer::Matrix44().PerspectiveFovLH(
			90.0f / 180.0f * 3.14f, (float)w / (float)h, 1.0f, 500.0f));
}

void KdEffekseerManager::Update()
{
	if (m_efkManager == nullptr) { return; }

	UpdateEffekseerEffect();

	UpdateEkfCameraMatrix();
}

void KdEffekseerManager::Draw()
{
	if (m_efkManager == nullptr ||
		m_efkRenderer == nullptr) {
		return;
	}

	m_efkRenderer->BeginRendering();
	m_efkManager->Draw();
	m_efkRenderer->EndRendering();
}

std::weak_ptr<KdEffekseerObject> KdEffekseerManager::Play(
	const std::string& effName, const DirectX::SimpleMath::Vector3& pos, const float size, const float speed, bool isLoop)
{
	PlayEfkInfo info;

	info.FileName	= effName;
	info.Pos		= pos;
	info.Size		= Math::Vector3(size);
	info.Speed		= speed;
	info.IsLoop		= isLoop;

	return Play(info);
}

void KdEffekseerManager::StopAllEffect()
{
	if (m_efkManager == nullptr) { return; }

	auto efkFoundItr = m_nowEffectPlayList.begin();
	while (efkFoundItr != m_nowEffectPlayList.end())
	{
		KdEffekseerObject* effObj = efkFoundItr->get();
		if (effObj)
		{
			if (effObj->IsLoop()) effObj->SetLoop(false);
		}
		++efkFoundItr;
	}

	m_efkManager->StopAllEffects();
}

void KdEffekseerManager::StopEffect(const std::string& name)
{
	if (m_efkManager == nullptr) return;

	// m_effectMap にある1つのオブジェクトだけだと最新インスタンスを指していない可能性があるため、
	// 実際に「現在再生中の個体」を全て走査して止める。
	for (auto& sp : m_nowEffectPlayList)
	{
		if (!sp) continue;
		const auto& info = sp->GetPlayEfkInfo();
		if (info.FileName == name)
		{
			sp->SetLoop(false);                  // ループ再生の再投入を防止
			m_efkManager->StopEffect(sp->GetHandle()); // 即停止
		}
	}
}

void KdEffekseerManager::StopEffect(const Effekseer::Handle& handle)
{
	m_efkManager->StopEffect(handle);
}

void KdEffekseerManager::Release()
{
	Reset();

	m_efkManager.Reset();
	m_efkRenderer.Reset();
}

void KdEffekseerManager::Reset()
{
	StopAllEffect();

	m_effectMap.clear();
	m_nowEffectPlayList.clear();

	m_isPause = false;
}

void KdEffekseerManager::SetPos(const int handle, const Math::Vector3& pos)
{
	Effekseer::Vector3D efkPos = GetEfkVec3D(pos);

	m_efkManager->SetLocation(handle, efkPos);
}

void KdEffekseerManager::SetRotation(const int handle, const Math::Vector3& axis, const float angle)
{
	Effekseer::Vector3D efkAxis = GetEfkVec3D(axis);

	m_efkManager->SetRotation(handle, efkAxis, angle);
}

void KdEffekseerManager::SetWorldMatrix(const int handle, const Math::Matrix& mWorld)
{
	Effekseer::Matrix43 mEfk{};

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			mEfk.Value[i][j] = mWorld.m[i][j];
		}
	}

	m_efkManager->SetMatrix(handle, mEfk);
}

void KdEffekseerManager::SetScale(const int handle, const Math::Vector3& scale)
{
	float scaleX = scale.x;
	float scaleY = scale.y;
	float scaleZ = scale.z;

	m_efkManager->SetScale(handle, scaleX, scaleY, scaleZ);
}

void KdEffekseerManager::SetScale(const int handle, const float scale)
{
	m_efkManager->SetScale(handle, scale, scale, scale);
}

void KdEffekseerManager::SetSpeed(const int handle, const float speed)
{
	m_efkManager->SetSpeed(handle, speed);
}

void KdEffekseerManager::SetPause(const int handle, const bool isPause)
{
	m_efkManager->SetPaused(handle, isPause);
}

void KdEffekseerManager::SetColor(const int handle, const Math::Vector4& color)
{
	m_efkManager->SetAllColor(
		handle,
		Effekseer::Color(
			static_cast<uint8_t>(color.x * 255.0f),
			static_cast<uint8_t>(color.y * 255.0f),
			static_cast<uint8_t>(color.z * 255.0f),
			static_cast<uint8_t>(color.w * 255.0f)));
}

const bool KdEffekseerManager::IsPlaying(const int handle) const
{
	return (m_efkManager->GetInstanceCount(handle) != 0);
}

std::weak_ptr<KdEffekseerObject> KdEffekseerManager::Play(const PlayEfkInfo& info)
{
	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	// 渡された座標をEffekseerの座標に置き換え
	Effekseer::Vector3D efkPos = GetEfkVec3D(info.Pos);

	Effekseer::Handle handle = 0;

	std::shared_ptr<KdEffekseerObject> spEfkObject = std::make_shared<KdEffekseerObject>();

	auto efkFoundItr = m_effectMap.find(info.FileName);
	// 既に生成されたことがある
	if (efkFoundItr != m_effectMap.end())
	{
		handle = m_efkManager->Play(efkFoundItr->second->GetEffect(), efkPos);
		spEfkObject->SetEffect(efkFoundItr->second->WorkEffect());
	}
	// エフェクト新規生成
	else
	{
		std::string loadFileName = info.FileName;

		// エフェクト新規生成
		auto effect = Effekseer::Effect::Create(m_efkManager,
			(const EFK_CHAR*)sjis_to_wide(loadFileName).c_str());

		if (effect == nullptr)
		{
#ifdef _DEBUG
			assert(0 && "Effekseerのエフェクト作成失敗");
#endif
			return std::weak_ptr<KdEffekseerObject>();
		}

		handle = m_efkManager->Play(effect, efkPos);
		spEfkObject->SetEffect(effect);
		m_effectMap[info.FileName] = spEfkObject;
	}

	// 自分で追加(ワールド行列を使うか判定)
	if (info.UseWorldMatrix)
	{
		SetWorldMatrix(handle, info.World);
		m_efkManager->SetSpeed(handle, info.Speed);
	}
	else
	{
		m_efkManager->SetScale(handle, info.Size.x, info.Size.y, info.Size.z);
		m_efkManager->SetSpeed(handle, info.Speed * deltaTime);
		Math::Vector3 rotate = ConvertToRadian(info.Rotate);
		m_efkManager->SetRotation(handle, rotate.x, rotate.y, rotate.z);
	}


	// エフェクトオブジェクト情報設定（ハンドルを先に設定してから色を適用する）
	spEfkObject->SetParentManager(m_efkManager);
	spEfkObject->SetHandle(handle);
	spEfkObject->SetColor(info.Color);
	spEfkObject->SetPlayEfkInfo(info);

	m_nowEffectPlayList.emplace_back(spEfkObject);
	return spEfkObject;
}

std::weak_ptr<KdEffekseerObject> KdEffekseerManager::Play(
	const std::shared_ptr<KdEffekseerObject>& spObject)
{
	return Play(spObject->GetPlayEfkInfo());
}

void KdEffekseerManager::UpdateEffekseerEffect()
{
	if (m_isPause) { return; }

	m_efkManager->Update();

	// ループ再生監視
	{
		// エフェクト再再生対象エフェクトリスト
		std::vector<PlayEfkInfo> replayList{};

		auto efkFoundItr = m_nowEffectPlayList.begin();
		while (efkFoundItr != m_nowEffectPlayList.end())
		{
			KdEffekseerObject* effObj = efkFoundItr->get();
			if (effObj)
			{
				int handle = effObj->GetHandle();
				// 再生が終了している
				if (m_efkManager->GetInstanceCount(handle) == 0)
				{
					bool isLoop = effObj->IsLoop();

					// ループ対象なら再再生リストに追加して後程全て再生させる
					if (isLoop)
					{
						const PlayEfkInfo& info = effObj->GetPlayEfkInfo();
						replayList.push_back(info);
					}

					// 再生終了したエフェクトはリストから除外する
					efkFoundItr->reset();

					// ハンドル値が変わるので今の再生リストから除外する
					efkFoundItr = m_nowEffectPlayList.erase(efkFoundItr);
					continue;
				}
			}

			++efkFoundItr;
		}

		// リプレイ対象エフェクトを全て再生させる
		for (auto&& efkInfo : replayList)
		{
			Play(efkInfo);
		}
		replayList.clear();
	}
}

void KdEffekseerManager::UpdateEkfCameraMatrix()
{
	std::shared_ptr<KdCamera> spCamera = m_wpCamera.lock();
	if (!spCamera) return;

	Math::Matrix mView = spCamera->GetCameraMatrix().Invert();
	Math::Matrix mProj = spCamera->GetProjMatrix();
	Effekseer::Matrix44 mEfkView;
	Effekseer::Matrix44 mEfkProj;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			mEfkView.Values[i][j] = mView.m[i][j];
			mEfkProj.Values[i][j] = mProj.m[i][j];
		}
	}

	m_efkRenderer->SetProjectionMatrix(mEfkProj);
	m_efkRenderer->SetCameraMatrix(mEfkView);
}

bool KdEffekseerObject::IsPlaying()
{
	if (m_parentManager == nullptr) { return false; }

	// ハンドルが0( 未再生 or 再生終了 )でない場合はTrue, 
	// そうでなければFalse
	return m_parentManager->GetInstanceCount(m_handle) != 0;
}

void KdEffekseerObject::SetPos(const Math::Vector3& pos)
{
	m_info.Pos = pos;

	KdEffekseerManager::GetInstance().SetPos(m_handle, pos);
}

void KdEffekseerObject::SetScale(const Math::Vector3 scale)
{
	m_info.Size = scale;

	KdEffekseerManager::GetInstance().SetScale(m_handle, scale);
}

void KdEffekseerObject::SetScale(const float scale)
{
	m_info.Size = Math::Vector3(scale);

	KdEffekseerManager::GetInstance().SetScale(m_handle, scale);
}

void KdEffekseerObject::SetSpeed(const float speed)
{
	m_info.Speed = speed;

	KdEffekseerManager::GetInstance().SetSpeed(m_handle, speed);
}

void KdEffekseerObject::SetWorldMatrix(const Math::Matrix& mWorld)
{
	KdEffekseerManager::GetInstance().SetWorldMatrix(m_handle, mWorld);
}

void KdEffekseerObject::SetColor(const Math::Vector4& color)
{
	KdEffekseerManager::GetInstance().SetColor(m_handle, color);
}
