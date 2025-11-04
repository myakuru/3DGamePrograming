#include "Timer.h"
#include"../../Utility/Time.h"
#include"../../../Scene/SceneManager.h"
#include"../../../main.h"
#include"../../../../MyFramework/Manager/JsonManager/JsonManager.h"

const uint32_t Timer::TypeID = KdGameObject::GenerateTypeID();

Timer::Timer()
{
	m_typeID = TypeID;
}

void Timer::Init()
{
	SelectDraw2DTexture::Init();

	m_srcRect = { 0, 0, 128, 256 }; // 1
	//m_srcRect = { 128, 0, 128, 256 }; // 2
	m_texture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Time/Digit01.png");
	m_resultTexture = KdAssets::Instance().m_textures.GetData("Asset/Textures/Time/ResultTime.png");
	m_notDraw = false; // 描画しないフラグを初期化
	m_displayTime = 0; // 初期表示時間を0に設定
	m_lastTime = 0.0f; // 最後の時間を初期化

	m_scale = { 0.015f, 0.15f, 1.0f };
	m_resultScaleInited = false;
}

void Timer::Update()
{
	// 残り秒数を取得
	float timeLeft = Time::Instance().GetCountdownTimeLeft();

	if (SceneManager::Instance().m_gameClear)
	{
		m_lastTime = timeLeft;
		return; // ゲームクリア時はタイマーを更新しない
	}

	float deltaTime = Application::Instance().GetUnscaledDeltaTime();

	if (SceneManager::Instance().IsBossAppear())
	{
		if (m_position.y > m_dawnPos.y) m_position.y -= m_dawnTimer * deltaTime;
	}

	
	m_displayTime = static_cast<int>(timeLeft);
	

	ResultTimerUpdate();

}

void Timer::ResultTimerUpdate()
{
	if (SceneManager::Instance().GetResultFlag())
	{
		float time = Time::Instance().GetElapsedTime();
		float deltaTime = Application::Instance().GetUnscaledDeltaTime();

		// リザルト開始時に一度だけ初期化
		if (!m_resultScaleInited)
		{
			m_scale = { 0.35f, 3.5f, 1.0f };
			m_resultScaleInited = true;
		}

		if (time >= 0.0f && time <= 2.0f)
		{
			// ランダムな秒数を設定
			m_displayTime = KdRandom::GetInt(3600, 36000);
		}
		else if (time >= 3.0f && time <= 5.0f)
		{

			if (time >= 4.6f)
			{
				if (m_scale.y >= 0.0f)
				{
					m_scale.y -= 50.0f * deltaTime;
				}
				else
				{
					m_scale.y = 0.0f;
					m_notDraw = true;
				}
			}
		}
	}
	else
	{
		m_notDraw = false; // ゲームクリア時以外は描画フラグをリセット
	}
}

void Timer::DrawSprite()
{
	if (SceneManager::Instance().IsIntroCamera()) return;

	if (SceneManager::Instance().GetResultFlag())
	{
		if (m_notDraw) return; // 描画しないフラグが立っている場合は何もしない

		// 現在のビューポートサイズ取得
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		const float sx = vp.width / kRefW;
		const float sy = vp.height / kRefH;

		Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

		KdShaderManager::Instance().m_spriteShader.SetMatrix(m_mWorld * uiScale);

		int time = std::min(m_displayTime, 99 * 60 * 60 + 99 * 60 + 99); //99:99:99

		int hours = time / 3600;
		int minutes = (time / 60) % 100;
		int seconds = time % 60;

		int digits[6] = {
			hours / 10,
			hours % 10,
			minutes / 10,
			minutes % 10,
			seconds / 10,
			seconds % 10
		};

		int baseX = static_cast<int>(m_position.x);
		int baseY = static_cast<int>(m_position.y);

		// コロン無しで数字6桁を連続表示（h, m, sの間に余白を追加）
		const int digitGap = 750;      // 通常の数字間隔
		const int extraGap = 200;       // 区切りごとの追加余白
		int offsetX = 0;

		for (int i = 0; i < 6; ++i)
		{
			int n = digits[i];
			int texIndex = n;
			Math::Rectangle srcRect = { 100 * texIndex, 0, 100, 100 };
			KdShaderManager::Instance().m_spriteShader.DrawTex(
				m_resultTexture,
				baseX + offsetX,
				baseY,
				&srcRect,
				&m_color
			);
			offsetX += digitGap;
			if (i == 1 || i == 3) offsetX += extraGap; // h→m, m→sの間に余白
		}
	}
	else
	{
		// 現在のビューポートサイズ取得
		Math::Viewport vp;
		KdDirect3D::Instance().CopyViewportInfo(vp);

		const float sx = vp.width / kRefW;
		const float sy = vp.height / kRefH;

		Math::Matrix uiScale = Math::Matrix::CreateScale(sx, sy, 1.0f);

		KdShaderManager::Instance().m_spriteShader.SetMatrix(m_mWorld * uiScale);

		int time = std::min(m_displayTime, 99 * 60 * 60 + 99 * 60 + 99); //99:99:99

		int hours = time / 3600;
		int minutes = (time / 60) % 100;
		int seconds = time % 60;

		int digits[6] = {
			hours / 10,
			hours % 10,
			minutes / 10,
			minutes % 10,
			seconds / 10,
			seconds % 10
		};

		int baseX = static_cast<int>(m_position.x);
		int baseY = static_cast<int>(m_position.y);

		// 通常通りコロン含めて表示
		for (int i = 0; i < 8; ++i)
		{
			if (i == 2 || i == 5)
			{
				// コロンの描画（2桁目と4桁目の後）
				Math::Rectangle colonRect = { 128 * 10, 0, 128, 256 };
				KdShaderManager::Instance().m_spriteShader.DrawTex(
					m_texture,
					baseX + 1280 * i,
					baseY,
					&colonRect,
					&m_color
				);
			}
			else
			{
				// 数字の描画
				int digitIdx = i - (i > 2 ? 1 : 0) - (i > 5 ? 1 : 0); // コロン分インデックス調整
				int n = digits[digitIdx];
				int texIndex = (n == 0) ? 9 : n - 1; // 0はインデックス9
				Math::Rectangle srcRect = { 128 * texIndex, 0, 128, 256 };
				KdShaderManager::Instance().m_spriteShader.DrawTex(
					m_texture,
					baseX + 1280 * i,
					baseY,
					&srcRect,
					&m_color
				);
			}
		}
	}

	KdShaderManager::Instance().m_spriteShader.SetMatrix(Math::Matrix::Identity);
}

void Timer::ImGuiInspector()
{
	SelectDraw2DTexture::ImGuiInspector();
	ImGui::DragFloat("Dawn Timer", &m_dawnTimer);
	ImGui::DragFloat3("Dawn Position", &m_dawnPos.x);
}

void Timer::JsonSave(nlohmann::json& _json) const
{
	SelectDraw2DTexture::JsonSave(_json);
	_json["DawnTimer"] = m_dawnTimer;
	_json["DawnPos"] = JSON_MANAGER.VectorToJson(m_dawnPos);
}

void Timer::JsonInput(const nlohmann::json& _json)
{
	SelectDraw2DTexture::JsonInput(_json);
	if (_json.contains("DawnTimer")) m_dawnTimer = _json["DawnTimer"].get<float>();
	if (_json.contains("DawnPos")) m_dawnPos = JSON_MANAGER.JsonToVector(_json["DawnPos"]);
}
