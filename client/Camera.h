#pragma once
#include"Vector2.h"

class Camera
{
public:
	Camera() = default;
	~Camera() = default;

	
	void reset() {
		position.x = 0;
		position.y = 0;
		size.x = 0;
		size.y = 0;
	}


	void on_update(int delta) {


		if (is_shaking) {
			position.x = (-50 + rand() % 100) / 50.0f * shaking_strength;
			position.y = (-50 + rand() % 100) / 50.0f * shaking_strength;
		}
	}
	void set_size(const Vector2 size) {
		

		this->size = size;
	}
	const Vector2& get_size() const {
		return size;
	}
	const Vector2& get_position()const {
		return position;
	}
	void set_position(const Vector2& pos) {
		
		position = pos;
	}
	void look_at(const Vector2& target) {
		position = target - size / 2.0f;
	}


private:
	Vector2 position;             //�����λ��
	Vector2 size;                 //�������С

	bool is_shaking = false;      //�Ƿ����ڶ���
	float shaking_strength = 0;   //�������������
};