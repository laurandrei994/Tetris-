#include "IBoard.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include "Utils.h"

FieldInfo::FieldInfo(sf::Texture& texture, uint16_t id)
{
	sf::IntRect rectangle{ (id % 7) * 18, 0, 18, 18 };
	m_Sprite.setTexture(texture);
	m_Sprite.setTextureRect(rectangle);
}

Field& Field::operator=(const Field& field)
{
	this->m_Occupied = field.m_Occupied;
	this->m_Visible = field.m_Visible;
	this->m_Info = field.m_Info;

	return *this;
}

IBoard::IBoard(Position size, IGame& game)
	: m_Game{ game }, m_Fields{}, m_FieldInfos{}, m_Size{ size }, m_ToBeCleaned{}, m_ElapsedTime{ 0.f }, m_ToRemoveBlocks{ false }
{
	for (int x = 0; x < size.x; x++)
		for (int y = 0; y < size.y; y++)
			m_Fields[Convert2DTo1D(x, y)] = std::make_unique<Field>();

	for (int id = 0; id < 7; id++)
		m_FieldInfos[id] = std::make_unique<FieldInfo>(m_Game.m_texture, id);
}

void IBoard::Update(const sf::Time& dt)
{
	MarkLinesForRemoval();
	if (m_ToRemoveBlocks)
	{
		m_ElapsedTime += dt.asSeconds();
		Blink();
		if (m_ElapsedTime > 0.6f)
		{
			m_ElapsedTime = 0.f;
			CleanLines();
		}
	}
}

void IBoard::Clean()
{
	for (uint16_t x = 0; x < m_Size.x; x++)
	{
		for (uint16_t y = 0; y < m_Size.y; y++)
		{
			auto field = GetField(x, y);
			field->m_Occupied = false;
			field->m_Visible = true;
			field->m_Info = nullptr;
		}
	}
}

void IBoard::AddBlock(uint16_t id, std::array<Position, 16> block)
{
	for (int i = 0; i < 16; i++)
	{
		auto field = GetField(block[i].x, block[i].y);
		field->m_Occupied = true;
		field->m_Info = m_FieldInfos[id].get();
	}
}

bool IBoard::IsOccupied(std::array<Position, 16> block)
{
	for (int i = 0; i < 16; i++)
	{
		auto field = GetField(block[i].x, block[i].y);
		if (field->m_Occupied)
			return true;
	}
	return false;
}

Field* IBoard::GetField(uint16_t x, uint16_t y)
{
	return m_Fields[Convert2DTo1D(x, y)].get();
}

void IBoard::Draw(sf::RenderWindow& window)
{
	for (int x = 0; x < m_Size.x; x++)
		for (int y = 0; y < m_Size.y; y++)
		{
			auto field = GetField(x, y);

			//if field isn't occupied yet, m_Info is set to nullptr
			if (field->m_Occupied && field->m_Visible)
			{
				field->m_Info->m_Sprite.setPosition(x * 18.f, y * 18.f);
				window.draw(field->m_Info->m_Sprite);
			}
		}
}


int IBoard::Convert2DTo1D(uint16_t x, uint16_t y)
{
	return (y * m_Size.x + x);
}

void IBoard::CleanLines()
{
	if (m_ToBeCleaned.empty())
		return;

	sf::Music clearLinesMusic;
	if (!clearLinesMusic.openFromFile("clear.wav"))
		std::cout << "Could not load clear.wav from file !! \n";

	//m_Game.m_gameplayMusic.stop();
	clearLinesMusic.play();

	for (auto i : m_ToBeCleaned)
	{
		for (auto y = i; y >= 0; y--)
		{
			for (auto x = 0; x < m_Size.x; x++)
			{
				int up = y - 1;
				if (up < 0)
					continue;
				*GetField(x, y) = *GetField(x, up);
			}
		}
	}
	m_ToBeCleaned.clear();
	m_ToRemoveBlocks = false;
	m_Game.m_gameplayMusic.play();
}

void IBoard::Blink()
{
	//Speeds up blinking

	int num = int(m_ElapsedTime * 5.f);
	for (auto y : m_ToBeCleaned)
		for (int x = 0; x < m_Size.x; x++)
			GetField(x, y)->m_Visible = (num % 2 != 0);
}