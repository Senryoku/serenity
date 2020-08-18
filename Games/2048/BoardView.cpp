/*
 * Copyright (c) 2020, the SerenityOS developers.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "BoardView.h"
#include <LibGUI/FontDatabase.h>
#include <LibGUI/Painter.h>
#include <LibGfx/Font.h>
#include <LibGfx/Palette.h>

BoardView::BoardView(const Game::Board* board)
    : m_board(board)
{
}

BoardView::~BoardView()
{
}

void BoardView::set_score(size_t score)
{
    if (m_score == score && !m_score_text.is_null())
        return;

    m_score = score;
    m_score_text = String::format("Score: %d", score);
    update();
}

void BoardView::set_board(const Game::Board* board)
{
    if (m_board == board)
        return;

    m_board = board;
    update();
}

void BoardView::pick_font()
{
    constexpr static auto liza_regular = "Liza Regular";
    String best_font_name = liza_regular;
    int best_font_size = -1;
    auto& font_database = GUI::FontDatabase::the();
    font_database.for_each_font([&](const StringView& font_name) {
        // Only consider variations of Liza Regular.
        if (!font_name.starts_with(liza_regular))
            return;
        auto metadata = font_database.get_metadata_by_name(font_name);
        if (!metadata.has_value())
            return;
        auto size = metadata.value().glyph_height;
        if (size * 2 <= m_cell_size && size > best_font_size) {
            best_font_name = font_name;
            best_font_size = size;
        }
    });

    auto font = font_database.get_by_name(best_font_name);
    set_font(font);
}

Gfx::IntRect BoardView::score_rect() const
{
    int score_width = font().width(m_score_text);
    return { 0, 2, score_width, font().glyph_height() };
}

size_t BoardView::rows() const
{
    if (!m_board)
        return 0;
    return m_board->size();
}

size_t BoardView::columns() const
{
    if (!m_board)
        return 0;
    if (m_board->is_empty())
        return 0;
    return (*m_board)[0].size();
}

void BoardView::resize_event(GUI::ResizeEvent&)
{
    int score_height = font().glyph_height() + 2;

    constexpr float padding_ratio = 7;
    m_padding = min(
        width() / (columns() * (padding_ratio + 1) + 1),
        (height() - score_height) / (rows() * (padding_ratio + 1) + 1));
    m_cell_size = m_padding * padding_ratio;

    pick_font();
}

void BoardView::keydown_event(GUI::KeyEvent& event)
{
    if (!on_move)
        return;

    switch (event.key()) {
    case KeyCode::Key_A:
    case KeyCode::Key_Left:
        on_move(Game::Direction::Left);
        break;
    case KeyCode::Key_D:
    case KeyCode::Key_Right:
        on_move(Game::Direction::Right);
        break;
    case KeyCode::Key_W:
    case KeyCode::Key_Up:
        on_move(Game::Direction::Up);
        break;
    case KeyCode::Key_S:
    case KeyCode::Key_Down:
        on_move(Game::Direction::Down);
        break;
    default:
        return;
    }
}

Gfx::Color BoardView::background_color_for_cell(u32 value)
{
    switch (value) {
    case 0:
        return Color::from_rgb(0xcdc1b4);
    case 2:
        return Color::from_rgb(0xeee4da);
    case 4:
        return Color::from_rgb(0xede0c8);
    case 8:
        return Color::from_rgb(0xf2b179);
    case 16:
        return Color::from_rgb(0xf59563);
    case 32:
        return Color::from_rgb(0xf67c5f);
    case 64:
        return Color::from_rgb(0xf65e3b);
    case 128:
        return Color::from_rgb(0xedcf72);
    case 256:
        return Color::from_rgb(0xedcc61);
    case 512:
        return Color::from_rgb(0xedc850);
    case 1024:
        return Color::from_rgb(0xedc53f);
    case 2048:
        return Color::from_rgb(0xedc22e);
    default:
        ASSERT_NOT_REACHED();
    }
}

Gfx::Color BoardView::text_color_for_cell(u32 value)
{
    if (value <= 4)
        return Color::from_rgb(0x776e65);
    return Color::from_rgb(0xf9f6f2);
}

void BoardView::paint_event(GUI::PaintEvent&)
{
    Color background_color = Color::from_rgb(0xbbada0);

    GUI::Painter painter(*this);

    if (!m_board) {
        painter.fill_rect(rect(), background_color);
        return;
    }
    auto& board = *m_board;

    painter.draw_text(score_rect(), m_score_text, font(), Gfx::TextAlignment::TopLeft, palette().color(ColorRole::BaseText));

    int score_height = font().glyph_height() + 2;

    Gfx::IntRect field_rect {
        0,
        0,
        static_cast<int>(m_padding + (m_cell_size + m_padding) * columns()),
        static_cast<int>(m_padding + (m_cell_size + m_padding) * rows())
    };
    field_rect.center_within({ 0, score_height, width(), height() - score_height });
    painter.fill_rect(field_rect, background_color);

    for (size_t column = 0; column < columns(); ++column) {
        for (size_t row = 0; row < rows(); ++row) {
            auto rect = Gfx::IntRect {
                field_rect.x() + m_padding + (m_cell_size + m_padding) * column,
                field_rect.y() + m_padding + (m_cell_size + m_padding) * row,
                m_cell_size,
                m_cell_size,
            };
            auto entry = board[row][column];
            painter.fill_rect(rect, background_color_for_cell(entry));
            if (entry > 0)
                painter.draw_text(rect, String::number(entry), font(), Gfx::TextAlignment::Center, text_color_for_cell(entry));
        }
    }
}
