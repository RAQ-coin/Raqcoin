#include "addresstablemodel.h"

#include "guiutil.h"
#include "walletmodel.h"

#include "wallet.h"
#include "base58.h"

#include <QFont>

#include "optionsmodel.h"
#include "raqcoinunits.h"

const QString AddressTableModel::Send = "S";
const QString AddressTableModel::Receive = "R";

struct AddressTableEntry
{
    enum Type {
        Sending,
        Receiving
    };

    Type type;
    QString label;
    QString address;
    qint64 balances;
    QString signType;
    bool isMiner;

    AddressTableEntry() {}
    AddressTableEntry(Type type, const QString &label, const QString &address, const qint64 &balances, const QString &signType, const bool &isMiner=false):
        type(type), label(label), address(address), balances(balances), signType(signType), isMiner(isMiner) {}
};

struct AddressTableEntryLessThan
{
    bool operator()(const AddressTableEntry &a, const AddressTableEntry &b) const
    {
        return a.address < b.address;
    }
    bool operator()(const AddressTableEntry &a, const QString &b) const
    {
        return a.address < b;
    }
    bool operator()(const QString &a, const AddressTableEntry &b) const
    {
        return a < b.address;
    }
};

// Private implementation
class AddressTablePriv
{
public:
    CWallet *wallet;
    QList<AddressTableEntry> cachedAddressTable;
    AddressTableModel *parent;

    AddressTablePriv(CWallet *wallet, AddressTableModel *parent):
        wallet(wallet), parent(parent) {}

    QString getSignTypeName(const CRaqcoinAddress& address) {
        char choised_sign_name[32] = {0};
        CKeyID keyid;
        if (address.GetKeyID(keyid)) {
            CPubKey pubkey;
            if (wallet->GetPubKey(keyid, pubkey)) {
                int config_value = pubkey.getPubKeyIndex();
                if (config_value >= 0) {
                    get_choised_config_from_config_value(config_value, choised_sign_name);    
                }
            }
        }

        QString signTypeName = choised_sign_name;
        if (address.ToString() == CRaqcoinAddress(wallet->vchDefaultKey.GetID()).ToString()) {
            signTypeName += "-default";
        }

        return signTypeName;
    }

    void refreshAddressTable()
    {
        cachedAddressTable.clear();
        {
            LOCK(wallet->cs_wallet);
            BOOST_FOREACH(const PAIRTYPE(CTxDestination, std::string)& item, wallet->mapAddressBook)
            {
                const CRaqcoinAddress& address = item.first;
                const std::string& strName = item.second;
                bool fMine = IsMine(*wallet, address.Get());

                qint64 bal = 0;
                QString signType = "";
                bool isMiner = false;
                if (fMine) {
                    bal = wallet->GetBalance(address.ToString());
                    
                    signType = getSignTypeName(address);
                } else {
                    if (address.ToString() == wallet->vchMinerAddress) {
                        isMiner = true;
                    }
                }

                cachedAddressTable.append(AddressTableEntry(fMine ? AddressTableEntry::Receiving : AddressTableEntry::Sending,
                    QString::fromStdString(strName), QString::fromStdString(address.ToString()),
                    bal, signType, isMiner
                ));
            }
        }
        // qLowerBound() and qUpperBound() require our cachedAddressTable list to be sorted in asc order
        qSort(cachedAddressTable.begin(), cachedAddressTable.end(), AddressTableEntryLessThan());
    }

    void updateEntry(const QString &address, const QString &label, bool isMine, int status)
    {
        // Find address / label in model
        QList<AddressTableEntry>::iterator lower = qLowerBound(
            cachedAddressTable.begin(), cachedAddressTable.end(), address, AddressTableEntryLessThan());
        QList<AddressTableEntry>::iterator upper = qUpperBound(
            cachedAddressTable.begin(), cachedAddressTable.end(), address, AddressTableEntryLessThan());
        int lowerIndex = (lower - cachedAddressTable.begin());
        int upperIndex = (upper - cachedAddressTable.begin());
        bool inModel = (lower != upper);
        AddressTableEntry::Type newEntryType = isMine ? AddressTableEntry::Receiving : AddressTableEntry::Sending;

        qint64 bal = 0;
        QString signType = "";
        bool isMiner = false;
        std::string strAddress = address.toStdString();
        if (isMine && (status == CT_NEW || status == CT_UPDATED)) {
            bal = wallet->GetBalance(strAddress);

            CRaqcoinAddress abcAddress(strAddress);
            signType = getSignTypeName(abcAddress);
        } else if (!isMine) {
            if (strAddress == wallet->vchMinerAddress) {
                isMiner = true;
            }
        }

        switch(status)
        {
        case CT_NEW:
            if(inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_NOW, but entry is already in model\n");
                break;
            }
            parent->beginInsertRows(QModelIndex(), lowerIndex, lowerIndex);
            cachedAddressTable.insert(lowerIndex, AddressTableEntry(newEntryType, label, address, bal, signType, isMiner));
            parent->endInsertRows();
            break;
        case CT_UPDATED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_UPDATED, but entry is not in model\n");
                break;
            }
            lower->type = newEntryType;
            lower->label = label;
            lower->balances = bal;
            lower->signType = signType;
            lower->isMiner = isMiner;

            parent->emitDataChanged(lowerIndex);
            break;
        case CT_DELETED:
            if(!inModel)
            {
                OutputDebugStringF("Warning: AddressTablePriv::updateEntry: Got CT_DELETED, but entry is not in model\n");
                break;
            }
            parent->beginRemoveRows(QModelIndex(), lowerIndex, upperIndex-1);
            cachedAddressTable.erase(lower, upper);
            parent->endRemoveRows();
            break;
        }
    }

    int size()
    {
        return cachedAddressTable.size();
    }

    AddressTableEntry *index(int idx)
    {
        if(idx >= 0 && idx < cachedAddressTable.size())
        {
            return &cachedAddressTable[idx];
        }
        else
        {
            return 0;
        }
    }
};

AddressTableModel::AddressTableModel(CWallet *wallet, WalletModel *parent, const QString &type) :
    QAbstractTableModel(parent),walletModel(parent),wallet(wallet),priv(0)
{
    if (type == Receive) {
        columns << tr("Label") << tr("Address") << tr("Balances") << tr("SignType");
    } else {
        columns << tr("Label") << tr("Address") << tr("IsMiner");
    }

    priv = new AddressTablePriv(wallet, this);
    priv->refreshAddressTable();
}

AddressTableModel::~AddressTableModel()
{
    delete priv;
}

int AddressTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return priv->size();
}

int AddressTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return columns.length();
}

QVariant AddressTableModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
        return QVariant();

    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    if(role == Qt::DisplayRole || role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Label:
            if(rec->label.isEmpty() && role == Qt::DisplayRole)
            {
                return tr("(no label)");
            }
            else
            {
                return rec->label;
            }
        case Address:
            return rec->address;
        case 2:     //Balances or IsMiner
            if (rec->type == AddressTableEntry::Receiving) {    //Balances
                if (walletModel && walletModel->getOptionsModel()) {
                    int unit = walletModel->getOptionsModel()->getDisplayUnit();
                    return RaqcoinUnits::formatWithUnit(unit, rec->balances);
                }
            } else if (rec->type == AddressTableEntry::Sending) {   //IsMiner
                if (rec->isMiner) {
                    return "✓";    //✓ ✔
                } else {
                    return " ";
                }
            }
        case SignType:
            return rec->signType;
        }
    }
    else if (role == Qt::FontRole)
    {
        QFont font;
        if(index.column() == Address)
        {
            font = GUIUtil::raqcoinAddressFont();
        }
        return font;
    }
    else if (role == TypeRole)
    {
        switch(rec->type)
        {
        case AddressTableEntry::Sending:
            return Send;
        case AddressTableEntry::Receiving:
            return Receive;
        default: break;
        }
    }
    return QVariant();
}

bool AddressTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if(!index.isValid())
        return false;
    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    editStatus = OK;

    if(role == Qt::EditRole)
    {
        switch(index.column())
        {
        case Label:
            // Do nothing, if old label == new label
            if(rec->label == value.toString())
            {
                editStatus = NO_CHANGES;
                return false;
            }
            wallet->SetAddressBookName(CRaqcoinAddress(rec->address.toStdString()).Get(), value.toString().toStdString());
            break;
        case Address:
            // Do nothing, if old address == new address
            if(CRaqcoinAddress(rec->address.toStdString()) == CRaqcoinAddress(value.toString().toStdString()))
            {
                editStatus = NO_CHANGES;
                return false;
            }
            // Refuse to set invalid address, set error status and return false
            else if(!walletModel->validateAddress(value.toString()))
            {
                editStatus = INVALID_ADDRESS;
                return false;
            }
            // Check for duplicate addresses to prevent accidental deletion of addresses, if you try
            // to paste an existing address over another address (with a different label)
            else if(wallet->mapAddressBook.count(CRaqcoinAddress(value.toString().toStdString()).Get()))
            {
                editStatus = DUPLICATE_ADDRESS;
                return false;
            }
            // Double-check that we're not overwriting a receiving address
            else if(rec->type == AddressTableEntry::Sending)
            {
                {
                    LOCK(wallet->cs_wallet);
                    // Remove old entry
                    wallet->DelAddressBookName(CRaqcoinAddress(rec->address.toStdString()).Get());
                    // Add new entry with new address
                    wallet->SetAddressBookName(CRaqcoinAddress(value.toString().toStdString()).Get(), rec->label.toStdString());
                }
            }
            break;
        }
        return true;
    }
    return false;
}

QVariant AddressTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
            return columns[section];
        }
    }
    return QVariant();
}

Qt::ItemFlags AddressTableModel::flags(const QModelIndex &index) const
{
    if(!index.isValid())
        return 0;
    AddressTableEntry *rec = static_cast<AddressTableEntry*>(index.internalPointer());

    Qt::ItemFlags retval = Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    // Can edit address and label for sending addresses,
    // and only label for receiving addresses.
    if(rec->type == AddressTableEntry::Sending ||
      (rec->type == AddressTableEntry::Receiving && index.column()==Label))
    {
        retval |= Qt::ItemIsEditable;
    }
    return retval;
}

QModelIndex AddressTableModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    AddressTableEntry *data = priv->index(row);
    if(data)
    {
        return createIndex(row, column, priv->index(row));
    }
    else
    {
        return QModelIndex();
    }
}

void AddressTableModel::refreshAddressTable()
{
    priv->refreshAddressTable();
}

void AddressTableModel::updateEntry(const QString &address, const QString &label, bool isMine, int status)
{
    // Update address book model from Raqcoin core
    priv->updateEntry(address, label, isMine, status);
}

QString AddressTableModel::addRow(const QString &type, const QString &label, const QString &address)
{
    std::string strLabel = label.toStdString();
    std::string strAddress = address.toStdString();

    editStatus = OK;

    if(type == Send)
    {
        if(!walletModel->validateAddress(address))
        {
            editStatus = INVALID_ADDRESS;
            return QString();
        }
        // Check for duplicate addresses
        {
            LOCK(wallet->cs_wallet);
            if(wallet->mapAddressBook.count(CRaqcoinAddress(strAddress).Get()))
            {
                editStatus = DUPLICATE_ADDRESS;
                return QString();
            }
        }
    }
    else if(type == Receive)
    {
        // Generate a new address to associate with given label
        WalletModel::UnlockContext ctx(walletModel->requestUnlock());
        if(!ctx.isValid())
        {
            // Unlock wallet failed or was cancelled
            editStatus = WALLET_UNLOCK_FAILURE;
            return QString();
        }

        CPubKey newKey;
        unsigned int default_config_value = 0;
        get_choised_info(NULL, NULL, NULL, &default_config_value);
        if(!wallet->GetKeyFromPool(newKey, default_config_value, false))
        {
            editStatus = KEY_GENERATION_FAILURE;
            return QString();
        }
        strAddress = CRaqcoinAddress(newKey.GetID()).ToString();
    }
    else
    {
        return QString();
    }

    // Add entry
    {
        LOCK(wallet->cs_wallet);
        wallet->SetAddressBookName(CRaqcoinAddress(strAddress).Get(), strLabel);
    }
    return QString::fromStdString(strAddress);
}

bool AddressTableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    Q_UNUSED(parent);
    AddressTableEntry *rec = priv->index(row);
    if(count != 1 || !rec || rec->type == AddressTableEntry::Receiving)
    {
        // Can only remove one row at a time, and cannot remove rows not in model.
        // Also refuse to remove receiving addresses.
        return false;
    }
    {
        LOCK(wallet->cs_wallet);
        wallet->DelAddressBookName(CRaqcoinAddress(rec->address.toStdString()).Get());
    }
    return true;
}

/* Look up label for address in address book, if not found return empty string.
 */
QString AddressTableModel::labelForAddress(const QString &address) const
{
    {
        LOCK(wallet->cs_wallet);
        CRaqcoinAddress address_parsed(address.toStdString());
        std::map<CTxDestination, std::string>::iterator mi = wallet->mapAddressBook.find(address_parsed.Get());
        if (mi != wallet->mapAddressBook.end())
        {
            return QString::fromStdString(mi->second);
        }
    }
    return QString();
}

int AddressTableModel::lookupAddress(const QString &address) const
{
    QModelIndexList lst = match(index(0, Address, QModelIndex()),
                                Qt::EditRole, address, 1, Qt::MatchExactly);
    if(lst.isEmpty())
    {
        return -1;
    }
    else
    {
        return lst.at(0).row();
    }
}

void AddressTableModel::emitDataChanged(int idx)
{
    emit dataChanged(index(idx, 0, QModelIndex()), index(idx, columns.length()-1, QModelIndex()));
}

bool AddressTableModel::setDefaultKey(const QString &address)
{
    CKeyID keyid;
    if (CRaqcoinAddress(address.toStdString()).GetKeyID(keyid)) {
        CPubKey pubKey;
        if (wallet->GetPubKey(keyid, pubKey)) {
            if (wallet->SetDefaultKey(pubKey)) {
                priv->refreshAddressTable();
                return true;
            }
        }
    }
    
    return false;
}

bool AddressTableModel::setMinerAddress(const QString &address)
{
    std::string strAddr = address.toStdString();
    if (wallet->SetMinerAddress(strAddr)) {
        priv->refreshAddressTable();
        return true;
    }
  
    return false;
}

QString AddressTableModel::getMinerAddress() {
    return QString::fromStdString(wallet->vchMinerAddress);
}