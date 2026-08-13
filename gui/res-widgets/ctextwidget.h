#ifndef CTEXTWIDGET_H
#define CTEXTWIDGET_H

#include <QFrame>
#include "gui/models/ctextdatamodel.h"

class CMyTableView;

namespace Ui {
class CTextWidget;
}

class CTextWidget : public QFrame
{
    Q_OBJECT

public:
    explicit CTextWidget(QWidget *parent = nullptr);
    ~CTextWidget() override;

    CMyTableView   * table_view();
    CTextDataModel * model();

    void  setCallbackAddEntry(std::function<bool(TextEntry &)> handler);
    void  setCallbackRemoveEntry(std::function<bool(std::vector<TextEntry> &)> handler);
    void  setCallbackMoveEntries(std::function<bool(std::vector<TextEntry> &, uint32_t)> handler);
    void  setCallbackUpdateEntry(std::function<bool(const TextEntry &)> handler);

    void  setCallbackEntrySecret(std::function<void(TextEntry&)> handler);
    void  setCallbackEntryPath(std::function<void(TextEntry&)> handler);
    void  setCallbackEntryUrl(std::function<void(TextEntry&)> handler);
    void  setCallbackEntryPlain(std::function<void(TextEntry&)> handler);

    void setCurrentNode(std::vector<TextEntry> & entries, const SNode & node, uint32_t loggedUserId); // to setup buttons
private slots:
    void slotAddEntry();
    void slotRemoveEntry();
    void slotContextTableMenu(const QPoint &pos);
    void  slotSetSecretEntry(TextEntry & entry);
    void  slotPathEntry(TextEntry & entry);
    void  slotUrlEntry(TextEntry & entry);
    void  slotSetPlainTextEntry(TextEntry & entry);
    void  slotEntryChanged(const TextEntry &entry);
    void  slotFileDialog(int row);
    void  slotFolderDialog(int row);
    void  slotGotoUrl(int row);
private:
    Ui::CTextWidget * ui;
    CTextDataModel  * m_model{nullptr};
    SNode   m_node{};
    uint32_t  m_loggedInUser{UINT32_MAX};

    std::function<bool(TextEntry &)>  m_cbAddEntry;
    std::function<bool(std::vector<TextEntry> &)>     m_cbRemoveEntry;
    std::function<bool(std::vector<TextEntry> &, uint32_t)> m_cbMoveEntries;
    std::function<bool(const TextEntry &)>  m_cbUpdateEntry;

    std::function<void(TextEntry&)> m_cbEntrySecret;
    std::function<void(TextEntry&)> m_cbEntryPath;
    std::function<void(TextEntry&)> m_cbEntryUrl;
    std::function<void(TextEntry&)> m_cbEntryPlain;

private: // methods
    void updateButtons();

};

#endif // CTEXTWIDGET_H
