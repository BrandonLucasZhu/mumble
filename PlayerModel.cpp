/* Copyright (C) 2005, Thorvald Natvig <thorvald@natvig.com>

   All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
   - Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation
     and/or other materials provided with the distribution.
   - Neither the name of the Mumble Developers nor the names of its
     contributors may be used to endorse or promote products derived from this
     software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "PlayerModel.h"
#include "MainWindow.h"
#include "Message.h"
#include "ServerHandler.h"
#include "Channel.h"
#include "Player.h"
#include "Global.h"
#ifdef Q_OS_WIN
#include "Overlay.h"
#endif

QHash <Channel *, ModelItem *> ModelItem::c_qhChannels;
QHash <Player *, ModelItem *> ModelItem::c_qhPlayers;

ModelItem::ModelItem(Channel *c) {
	this->cChan = c;
	this->pPlayer = NULL;
	c_qhChannels[c] = this;
}

ModelItem::ModelItem(Player *p) {
	this->cChan = NULL;
	this->pPlayer = p;
	c_qhPlayers[p] = this;
}

ModelItem::~ModelItem() {
	Q_ASSERT(qlPlayers.count() == 0);
	Q_ASSERT(qlChannels.count() == 0);

	if (cChan)
		c_qhChannels.remove(cChan);
	if (pPlayer)
		c_qhPlayers.remove(pPlayer);
}

ModelItem *ModelItem::parent() const {
	Channel *p;

	if (cChan)
		p = cChan->cParent;
	else
		p = pPlayer->cChannel;

	return c_qhChannels.value(p);
}

ModelItem *ModelItem::child(int idx) const {
	if (! validRow(idx))
		return NULL;

	if (idx < qlChannels.count())
		return c_qhChannels.value(channelAt(idx));
	else
		return c_qhPlayers.value(playerAt(idx));
}

bool ModelItem::validRow(int idx) const {
	return ((idx >= 0) && (idx < (qlPlayers.count() + qlChannels.count())));
}

Player *ModelItem::playerAt(int idx) const {
	idx -= qlChannels.count();
	if ((idx>= 0) && (idx < qlPlayers.count()))
		return qlPlayers.at(idx);
	return NULL;
}

Channel *ModelItem::channelAt(int idx) const {
	if ((idx>= 0) && (idx < qlChannels.count()))
		return qlChannels.at(idx);
	return NULL;
}

int ModelItem::rowOf(Channel *c) const {
	return qlChannels.lastIndexOf(c);
}

int ModelItem::rowOf(Player *p) const {
	int v = qlPlayers.lastIndexOf(p);
	if (v != -1)
		v += qlChannels.count();
	return v;
}

int ModelItem::rowOfSelf() const {
	ModelItem *p = parent();

	Q_ASSERT(p);

	if (pPlayer)
		return p->rowOf(pPlayer);
	else
		return p->rowOf(cChan);
}

int ModelItem::rows() const {
	return qlPlayers.count() + qlChannels.count();
}

int ModelItem::insertIndex(Channel *c) const {
	QList<QString> qls;
	Channel *cp;

	foreach(cp, qlChannels)
		qls << cp->qsName;
	qls << c->qsName;
	qSort(qls);

	return qls.lastIndexOf(c->qsName);
}

int ModelItem::insertIndex(Player *p) const {
	QList<QString> qls;
	Player *pp;

	foreach(pp, qlPlayers)
		qls << pp->qsName;
	qls << p->qsName;
	qSort(qls);

	return qls.lastIndexOf(p->qsName) + qlChannels.count();
}

void ModelItem::insertChannel(Channel *c) {
	int idx = insertIndex(c);
	qlChannels.insert(idx, c);
}

void ModelItem::insertPlayer(Player *p) {
	int idx = insertIndex(p) - qlChannels.count();
	qlPlayers.insert(idx, p);
}

PlayerModel::PlayerModel(QObject *p) : QAbstractItemModel(p) {
	qiTalkingOn=QIcon(":/icons/talking_on.png");
	qiTalkingAlt=QIcon(":/icons/talking_alt.png");
	qiTalkingOff=QIcon(":/icons/talking_off.png");
	qiMutedSelf=QIcon(":/icons/muted_self.png");
	qiMutedServer=QIcon(":/icons/muted_server.png");
	qiMutedLocal=QIcon(":/icons/muted_local.png");
	qiDeafenedSelf=QIcon(":/icons/deafened_self.png");
	qiDeafenedServer=QIcon(":/icons/deafened_server.png");
	qiAuthenticated=QIcon(":/icons/authenticated.png");
	qiChannel=QIcon(":/icons/channel.png");
	qiLinkedChannel=QIcon(":/icons/channel_linked.png");

	miRoot = new ModelItem(Channel::get(0));
}

PlayerModel::~PlayerModel() {
	removeAll();
	Q_ASSERT(ModelItem::c_qhPlayers.count() == 0);
	Q_ASSERT(ModelItem::c_qhChannels.count() == 1);
}


int PlayerModel::columnCount(const QModelIndex &) const
{
	return 2;
}

QModelIndex PlayerModel::index(int row, int column, const QModelIndex &p) const
{
	ModelItem *item;
	QModelIndex idx = QModelIndex();

	if (row == -1) {
		return QModelIndex();
	}

	if ( ! p.isValid()) {
		item = miRoot;
	} else {
        item = static_cast<ModelItem *>(p.internalPointer());
	}

	if (! item->validRow(row))
		return idx;

	idx = createIndex(row, column, item->child(row));

    return idx;
}

QModelIndex PlayerModel::index(Player *p, int column) const
{
	ModelItem *item = ModelItem::c_qhPlayers.value(p);
	Q_ASSERT(p);
	Q_ASSERT(item);
	QModelIndex idx=createIndex(item->rowOfSelf(), column, item);
	return idx;
}

QModelIndex PlayerModel::index(Channel *c) const
{
	ModelItem *item = ModelItem::c_qhChannels.value(c);
	Q_ASSERT(c);
	Q_ASSERT(item);
	if (!c || ! c->parent())
		return QModelIndex();
	QModelIndex idx=createIndex(item->rowOfSelf(), 0, item);
	return idx;
}

QModelIndex PlayerModel::parent(const QModelIndex &idx) const
{
	if (! idx.isValid())
	    return QModelIndex();

    ModelItem *item = static_cast<ModelItem *>(idx.internalPointer());

	ModelItem *pitem = item->parent();
	ModelItem *gpitem = (pitem) ? pitem->parent() : NULL;

	if (! pitem || ! gpitem)
		return QModelIndex();

	QModelIndex pidx = createIndex(pitem->rowOfSelf(), 0, pitem);


    return pidx;
}

int PlayerModel::rowCount(const QModelIndex &p) const
{
    ModelItem *item;

    int val = 0;

    if (!p.isValid())
        item = miRoot;
    else
        item = static_cast<ModelItem *>(p.internalPointer());

	val = item->rows();

	return val;
}

QString PlayerModel::stringIndex(const QModelIndex &idx) const
{
	ModelItem *item = static_cast<ModelItem *>(idx.internalPointer());
	if (!idx.isValid())
		return QString("invIdx");
	if (!item)
		return QString("invPtr");
	if (item->pPlayer)
		return QString("P:%1 [%2,%3]").arg(item->pPlayer->qsName).arg(idx.row()).arg(idx.column());
	else
		return QString("C:%1 [%2,%3]").arg(item->cChan->qsName).arg(idx.row()).arg(idx.column());
}

QVariant PlayerModel::data(const QModelIndex &idx, int role) const
{
    if (!idx.isValid())
        return QVariant();

	ModelItem *item = static_cast<ModelItem *>(idx.internalPointer());

	Channel *c = item->cChan;
	Player *p = item->pPlayer;

	if (!c && !p)
		return QVariant();

	QVariant v = otherRoles(idx.column(), role, (p != NULL));
	if (v.isValid())
		return v;

	QList<QVariant> l;

	if (p) {
		switch (role) {
			case Qt::DecorationRole:
				if (idx.column() == 0)
					return (p->bTalking) ? (p->bAltSpeak ? qiTalkingAlt : qiTalkingOn) : qiTalkingOff;
				break;
			case Qt::FontRole:
				if ((idx.column() == 0) && (p->sId == g.sId)) {
					QFont f = g.mw->font();
					f.setBold(true);
					return f;
				}
				break;
			case Qt::DisplayRole:
				if (idx.column() == 0)
					return p->qsName;
				if (p->iId >= 0)
					l << qiAuthenticated;
				if (p->bMute)
					l << qiMutedServer;
				if (p->bDeaf)
					l << qiDeafenedServer;
				if (p->bSelfMute)
					l << qiMutedSelf;
				if (p->bSelfDeaf)
					l << qiDeafenedSelf;
				if (p->bLocalMute)
					l << qiMutedLocal;
				return l;
			default:
				break;
		}
	} else {
		switch (role) {
			case Qt::DecorationRole:
				if (idx.column() == 0) {
					return qsLinked.contains(c) ? qiLinkedChannel : qiChannel;
				}
			case Qt::DisplayRole:
				if (idx.column() == 0)
					return c->qsName;
			default:
				break;
		}
	}
	return QVariant();
}

Qt::ItemFlags PlayerModel::flags(const QModelIndex &idx) const
{
	if (!idx.isValid())
		return 0;

    if (idx.column() != 0)
        return Qt::ItemIsEnabled | Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled;

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled;
}

QVariant PlayerModel::otherRoles(int section, int role, bool isPlayer) const
{
	switch(role) {
		case Qt::ToolTipRole:
			switch(section) {
				case 0:
					return isPlayer ? tr("Name of player") : tr("Name of channel");
				case 1:
					return isPlayer ? tr("Player flags") : QVariant();
			}
			break;
		case Qt::WhatsThisRole:
			switch(section) {
				case 0:
					if (isPlayer)
 						return tr("This is a player connected to the server. The icon to the left of the player indicates "
							"whether or not they are talking:<br />"
							"<img src=\":/icons/talking_on.png\" /> Talking<br />"
							"<img src=\":/icons/talking_off.png\" /> Not talking"
							);
					else
						return tr("This is a channel on the server. Only players in the same channel can hear each other.");
				case 1:
					return tr("This shows the flags the player has on the server, if any:<br />"
								"<img src=\":/icons/authenticated.png\" />Authenticated user<br />"
								"<img src=\":/icons/muted_self.png\" />Muted (by self)<br />"
								"<img src=\":/icons/muted_server.png\" />Muted (by admin)<br />"
								"<img src=\":/icons/deafened_self.png\" />Deafened (by self)<br />"
								"<img src=\":/icons/deafened_server.png\" />Deafened (by admin)<br />"
								"A player muted by himself is probably just away, talking on the phone or something like that.<br />"
								"A player muted by an admin is probably also just away, and the noise the player is making was annoying "
								"enough that an admin muted him.");
			}
			break;
	}
	return QVariant();
}

QVariant PlayerModel::headerData(int section, Qt::Orientation orientation,
                               int role) const
{
	if (orientation != Qt::Horizontal)
		return QVariant();

	switch(role) {
		case Qt::DisplayRole:
			switch(section) {
				case 0:
					return QString("Name");
				case 1:
					return QString("Flags");
			}
	}

    return QVariant();
}

void PlayerModel::unbugHide(const QModelIndex &idx) {
	QAbstractItemView *v=g.mw->qtvPlayers;
	QItemSelectionModel *sel=v->selectionModel();

	// If we hide the current item and re-show it as a child
	// an item that is nonexpanded, and then expand
	// that item, we get "interresting results".
	//
	// A permanent fix for this would be to let the private
	// pointer of the index be a struct with channel/player,
	// and recalculate the correct (sorted) position for each and
	// every use.

	if (sel->isSelected(idx) || (idx == v->currentIndex())) {
		v->clearSelection();
		v->setCurrentIndex(QModelIndex());
	}
}

void PlayerModel::hidePlayer(Player *p) {
	Channel *c = p->cChannel;
	ModelItem *item = ModelItem::c_qhChannels.value(c);

	int row = item->rowOf(p);

	unbugHide(index(p));

	beginRemoveRows(index(c), row, row);
	c->removePlayer(p);
	item->qlPlayers.removeAll(p);
	endRemoveRows();

	if (g.sId && (p->cChannel == Player::get(g.sId)->cChannel))
		updateOverlay();

	p->cChannel = NULL;
}

void PlayerModel::showPlayer(Player *p, Channel *c) {
	ModelItem *item = ModelItem::c_qhChannels.value(c);

	Q_ASSERT(p);
	Q_ASSERT(c);
	Q_ASSERT(item);

	int row = item->insertIndex(p);

	beginInsertRows(index(c), row, row);
	c->addPlayer(p);
	item->insertPlayer(p);
	endInsertRows();

	if (g.sId && (p->cChannel == Player::get(g.sId)->cChannel))
		updateOverlay();

	ensureSelfVisible();
}

void PlayerModel::ensureSelfVisible() {
	QStack<Channel *> chans;

	if (! g.sId)
		return;
	Channel *c = Player::get(g.sId)->cChannel;
	while (c) {
		chans.push(c);
		c = c->cParent;
	}
	while (! chans.isEmpty()) {
		c = chans.pop();
		g.mw->qtvPlayers->setExpanded(index(c), true);
	}
	recheckLinks();
}

void PlayerModel::recheckLinks() {
	if (! g.sId)
		return;

	bool bChanged = false;

	Channel *home = Player::get(g.sId)->cChannel;

	QSet<Channel *> all = home->allLinks();

	if (all == qsLinked)
		return;

	QSet<Channel *> changed = (all - qsLinked);
	changed += (qsLinked - all);

	qsLinked = all;

	foreach(Channel *c, changed) {
		QModelIndex idx = index(c);
		emit dataChanged(idx, idx);
		bChanged = true;
	}
	if (bChanged)
		updateOverlay();
}

Player *PlayerModel::addPlayer(short id, QString name) {
	Player *p = Player::add(id, this);
	p->qsName = name;

	new ModelItem(p);

	connect(p, SIGNAL(talkingChanged(bool)), this, SLOT(playerTalkingChanged(bool)));
	connect(p, SIGNAL(muteDeafChanged()), this, SLOT(playerMuteDeafChanged()));

	showPlayer(p, Channel::get(0));

	return p;
}

void PlayerModel::removePlayer(Player *p) {
	ModelItem *item = ModelItem::c_qhPlayers.value(p);

	hidePlayer(p);

	Player::remove(p);
	delete p;
	delete item;
}

void PlayerModel::movePlayer(Player *p, int id) {
	Channel *np = Channel::get(id);
	hidePlayer(p);
	showPlayer(p, np);
}

void PlayerModel::renamePlayer(Player *p, QString name) {
	Channel *c = p->cChannel;
	hidePlayer(p);
	p->qsName = name;
	showPlayer(p, c);
}

void PlayerModel::showChannel(Channel *c, Channel *p) {
	ModelItem *item = ModelItem::c_qhChannels.value(p);

	Q_ASSERT(p);
	Q_ASSERT(c);
	Q_ASSERT(item);

	int row = item->insertIndex(c);

	beginInsertRows(index(p), row, row);
	p->addChannel(c);
	item->insertChannel(c);
	endInsertRows();

	item = ModelItem::c_qhChannels.value(c);

	ensureSelfVisible();
}

void PlayerModel::hideChannel(Channel *c) {
	Channel *p = c->cParent;
	ModelItem *item = ModelItem::c_qhChannels.value(p);

	int row = item->rowOf(c);

	unbugHide(index(c));

	beginRemoveRows(index(p), row, row);
	p->removeChannel(c);
	item->qlChannels.removeAll(c);
	qsLinked.remove(c);
	endRemoveRows();

	item = ModelItem::c_qhChannels.value(c);
}

Channel *PlayerModel::addChannel(int id, Channel *p, QString name) {
	Channel *c = Channel::add(id, name, NULL);

	new ModelItem(c);

	showChannel(c, p);
	return c;
}

void PlayerModel::removeChannel(Channel *c) {
	ModelItem *item;
	Player *pl;
	Channel *subc;

	item=ModelItem::c_qhChannels.value(c);

	foreach(subc, item->qlChannels)
		removeChannel(subc);

	foreach(pl, item->qlPlayers)
		removePlayer(pl);

	hideChannel(c);
	Channel::remove(c);

	delete item;
	delete c;
}

void PlayerModel::moveChannel(Channel *c, int id) {
	Channel *np = Channel::get(id);
	hideChannel(c);
	showChannel(c, np);
}

void PlayerModel::linkChannels(Channel *c, QList<Channel *> links) {
	foreach(Channel *l, links)
		c->link(l);
	recheckLinks();
}

void PlayerModel::unlinkChannels(Channel *c, QList<Channel *> links) {
	foreach(Channel *l, links)
		c->unlink(l);
	recheckLinks();
}

void PlayerModel::unlinkAll(Channel *c) {
	c->unlink(NULL);
	recheckLinks();
}

void PlayerModel::removeAll() {
	ModelItem *item = miRoot;

	while (item->qlChannels.count() > 0)
		removeChannel(item->qlChannels[0]);

	while (item->qlPlayers.count() > 0)
		removePlayer(item->qlPlayers[0]);

	qsLinked.clear();
}

Player *PlayerModel::getPlayer(const QModelIndex &idx) const
{
	if (! idx.isValid())
		return NULL;

    ModelItem *item;
    item = static_cast<ModelItem *>(idx.internalPointer());

    return item->pPlayer;
}

Channel *PlayerModel::getChannel(const QModelIndex &idx) const
{
	if (! idx.isValid())
		return NULL;

    ModelItem *item;
    item = static_cast<ModelItem *>(idx.internalPointer());

	if (item->pPlayer)
		return item->pPlayer->cChannel;
	else
	    return item->cChan;
}

Channel *PlayerModel::getSubChannel(Channel *p, int idx) const
{
	ModelItem *item=ModelItem::c_qhChannels.value(p);
	if (idx < 0 || idx >= item->qlChannels.count())
		return NULL;
	return item->qlChannels.at(idx);
}


void PlayerModel::playerTalkingChanged(bool bTalking)
{
	Player *p=static_cast<Player *>(sender());
	QModelIndex idx = index(p);
	emit dataChanged(idx, idx);
//	if (g.sId && (p->cChannel == Player::get(g.sId)->cChannel))
	// If we can hear them, we need to update them
	updateOverlay();
}

void PlayerModel::playerMuteDeafChanged()
{
	Player *p=static_cast<Player *>(sender());
	QModelIndex idx = index(p, 1);
	emit dataChanged(idx, idx);
}

Qt::DropActions PlayerModel::supportedDropActions() const {
	return Qt::CopyAction;
}

QStringList PlayerModel::mimeTypes() const {
	QStringList sl;
	sl << "mumble/dragentry";
	return sl;
}

QMimeData *PlayerModel::mimeData(const QModelIndexList &idxs) const {
	QModelIndex idx;
	QByteArray qba;
	QDataStream ds(&qba, QIODevice::WriteOnly);

	foreach(idx, idxs) {
		Player *p = getPlayer(idx);
		Channel *c = getChannel(idx);
		if (p) {
			ds << false;
			ds << p->sId;
		} else {
			ds << true;
			ds << c->iId;
		}
	}
	QMimeData *md = new QMimeData();
	md->setData("mumble/dragentry", qba);
	return md;
}

bool PlayerModel::dropMimeData (const QMimeData *md, Qt::DropAction action, int row, int column, const QModelIndex &p) {
	if (! md->hasFormat(mimeTypes().at(0)))
		return false;

	QByteArray qba = md->data(mimeTypes().at(0));
	QDataStream ds(qba);

	bool isChannel;
	int iId = -1;
	short sId = -1;
	ds >> isChannel;

	if (isChannel)
		ds >> iId;
	else
		ds >> sId;

	Channel *c;
	if ( ! p.isValid()) {
		c = Channel::get(0);
	} else {
		c = getChannel(p);
	}

	if (! isChannel) {
		MessagePlayerMove mpm;
		mpm.sVictim = sId;
		mpm.iChannelId = c->iId;
		g.sh->sendMessage(&mpm);
	} else {
		MessageChannelMove mcm;
		mcm.iId = iId;
		mcm.iParent = c->iId;
		g.sh->sendMessage(&mcm);
	}

	return true;
}

void PlayerModel::updateOverlay() const {
#ifdef Q_OS_WIN
	if (g.sId) {
		Channel *c = Player::get(g.sId)->cChannel;
		ModelItem *item = ModelItem::c_qhChannels.value(c);
		if (item) {
			g.o->setPlayers(item->qlPlayers);
			return;
		}
	}
	QList<Player *> empty;
	g.o->setPlayers(empty);
#endif
}

PlayerDelegate::PlayerDelegate(QObject *p) : QItemDelegate(p) {
}

QSize PlayerDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == 1) {
		const QAbstractItemModel *m = index.model();
		QVariant data = m->data(index);
		QList<QVariant> ql = data.toList();
		return QSize(18 * ql.count(), 18);
	} else {
		return QItemDelegate::sizeHint(option, index);
	}
}

void PlayerDelegate::paint(QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index) const {
	if (index.column() == 1) {
		const QAbstractItemModel *m = index.model();
		QVariant data = m->data(index);
		QList<QVariant> ql = data.toList();

		painter->save();
		for(int i=0;i<ql.size();i++) {
			QRect r = option.rect;
			r.setSize(QSize(16,16));
			r.translate(i*18+1,1);
			drawDecoration(painter, option, r, qvariant_cast<QIcon>(ql[i]).pixmap(QSize(16,16)));
		}
		painter->restore();
		return;
	} else {
		QItemDelegate::paint(painter,option,index);
	}
}
