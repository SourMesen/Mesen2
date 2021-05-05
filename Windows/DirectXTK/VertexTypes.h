//--------------------------------------------------------------------------------------
// File: VertexTypes.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
//--------------------------------------------------------------------------------------

#pragma once

#if defined(_XBOX_ONE) && defined(_TITLE)
#include <d3d11_x.h>
#else
#include <d3d11_1.h>
#endif

#include <cstdint>

#include <DirectXMath.h>


namespace DirectX
{
    // Vertex struct holding position, color, and texture mapping information.
    struct VertexPositionColorTexture
    {
        VertexPositionColorTexture() = default;

        VertexPositionColorTexture(const VertexPositionColorTexture&) = default;
        VertexPositionColorTexture& operator=(const VertexPositionColorTexture&) = default;

        VertexPositionColorTexture(VertexPositionColorTexture&&) = default;
        VertexPositionColorTexture& operator=(VertexPositionColorTexture&&) = default;

        VertexPositionColorTexture(XMFLOAT3 const& iposition, XMFLOAT4 const& icolor, XMFLOAT2 const& itextureCoordinate) noexcept
            : position(iposition),
            color(icolor),
            textureCoordinate(itextureCoordinate)
        { }

        VertexPositionColorTexture(FXMVECTOR iposition, FXMVECTOR icolor, FXMVECTOR itextureCoordinate) noexcept
        {
            XMStoreFloat3(&this->position, iposition);
            XMStoreFloat4(&this->color, icolor);
            XMStoreFloat2(&this->textureCoordinate, itextureCoordinate);
        }

        XMFLOAT3 position;
        XMFLOAT4 color;
        XMFLOAT2 textureCoordinate;

        static constexpr unsigned int InputElementCount = 3;
        static const D3D11_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

}
