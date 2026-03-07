# Metal ZBIAS Implementation
To implement `D3DRS_ZBIAS`:
1. Track it in `m_RenderStates`.
2. Apply it in `ApplyPerDrawState()` inside `MetalDevice8.mm`.
