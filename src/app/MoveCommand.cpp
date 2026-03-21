// #include "app/MoveCommand.h"

// MoveCommand::MoveCommand(HullModel* model, int index, double dx, double dy, double dz)
//     : m_model(model)
//     , m_index(index)
//     , m_dx(dx)
//     , m_dy(dy)
//     , m_dz(dz)
// {
// }

// void MoveCommand::execute()
// {
//     m_model->movePoint(m_index, m_dx, m_dy, m_dz);
// }

// void MoveCommand::undo()
// {
//     // Reverse the displacement exactly.
//     m_model->movePoint(m_index, -m_dx, -m_dy, -m_dz);
// }