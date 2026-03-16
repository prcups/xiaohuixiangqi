#include "record.h"

bool Record::operator == (const Record &x)const
{
    if (fromX != x.fromX || fromY != x.fromY
            || toX != x.toX || toY != x.toY
       ) return false;
    if (ifEat != x.ifEat || ifJiangjun != x.ifJiangjun) return false;
    if (ifEat) {
        if (dstColor != x.dstColor || dstType != x.dstType)
            return false;
    }
    if (x.fenStr != fenStr) return false;
    return true;
}

QString Record::ToMoveString()
{
    QString s;
    s.append(QChar('a' + fromY));
    s.append(QChar('0' + fromX));
    s.append(QChar('a' + toY));
    s.append(QChar('0' + toX));
    return s;
}

uint qHash(const Record &key)
{
    int p = (1 << 25) + 7;
    int ans = 1;
    for (auto &i : key.fenStr)
        ans = ((long long)(ans + ans * i.toLatin1()) << 1) % p;
    ans = ((long long)(ans + ans * key.fromX) << 1) % p;
    ans = ((long long)(ans + ans * key.fromY) << 1) % p;
    ans = ((long long)(ans + ans * key.toX) << 1) % p;
    ans = ((long long)(ans + ans * key.toY) << 1) % p;
    ans = ((long long)(ans + ans * key.ifEat) << 1) % p;
    ans = ((long long)(ans + ans * key.ifJiangjun) << 1) % p;
    return ans;
}
