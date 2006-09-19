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

#ifndef _MAINWINDOW_H
#define _MAINWINDOW_H

class TrayIcon;
class ACLEditor;
class BanEditor;
class ServerHandler;
class GlobalShortcut;
class TextToSpeech;
class PlayerModel;

class MainWindow : public QMainWindow {
	friend class PlayerModel;
	Q_OBJECT
	public:
		PlayerModel *pmModel;
		QMenu *qmServer, *qmPlayer, *qmChannel, *qmAudio, *qmConfig, *qmHelp;
		QAction *qaServerConnect, *qaServerDisconnect, *qaServerBanList;
		QAction *qaPlayerKick, *qaPlayerBan, *qaPlayerMute, *qaPlayerDeaf, *qaPlayerLocalMute;
		QAction *qaAudioReset, *qaAudioMute, *qaAudioDeaf, *qaAudioTTS, *qaAudioStats, *qaAudioUnlink;
		QAction *qaConfigDialog, *qaConfigShortcuts;
		QAction *qaHelpWhatsThis, *qaHelpAbout, *qaHelpAboutSpeex, *qaHelpAboutQt, *qaHelpVersionCheck;
		QAction *qaChannelAdd, *qaChannelRemove, *qaChannelACL, *qaChannelLink, *qaChannelUnlink, *qaChannelUnlinkAll;
		QSplitter *qsSplit;

		GlobalShortcut *gsPushTalk, *gsResetAudio, *gsMuteSelf, *gsDeafSelf;
		GlobalShortcut *gsUnlink, *gsCenterPos, *gsPushMute, *gsMetaChannel, *gsToggleOverlay;
		GlobalShortcut *gsAltTalk;

		TrayIcon *ti;

		ACLEditor *aclEdit;
		BanEditor *banEdit;

		void recheckTTS();
		void appendLog(QString entry);
	protected:
		QTextEdit *qteLog;
		QTreeView *qtvPlayers;
		void createActions();
		void setupGui();
		void customEvent(QEvent *evt);
		virtual void closeEvent(QCloseEvent *e);
	public slots:
		void on_Players_customContextMenuRequested(const QPoint &pos);
		void on_Players_doubleClicked(const QModelIndex &idx);
		void on_ServerConnect_triggered();
		void on_ServerDisconnect_triggered();
		void on_ServerBanList_triggered();
		void on_PlayerMenu_aboutToShow();
		void on_PlayerKick_triggered();
		void on_PlayerBan_triggered();
		void on_PlayerMute_triggered();
		void on_PlayerDeaf_triggered();
		void on_PlayerLocalMute_triggered();
		void on_ChannelMenu_aboutToShow();
		void on_ChannelAdd_triggered();
		void on_ChannelRemove_triggered();
		void on_ChannelACL_triggered();
		void on_ChannelLink_triggered();
		void on_ChannelUnlink_triggered();
		void on_ChannelUnlinkAll_triggered();
		void on_AudioReset_triggered();
		void on_AudioMute_triggered();
		void on_AudioDeaf_triggered();
		void on_AudioTextToSpeech_triggered();
		void on_AudioUnlink_triggered();
		void on_AudioStats_triggered();
		void on_ConfigShortcuts_triggered();
		void on_ConfigDialog_triggered();
		void on_HelpWhatsThis_triggered();
		void on_HelpAbout_triggered();
		void on_HelpAboutSpeex_triggered();
		void on_HelpAboutQt_triggered();
		void on_HelpVersionCheck_triggered();
		void on_PushToTalk_triggered(bool);
		void on_PushToMute_triggered(bool);
		void on_AltPushToTalk_triggered(bool);
		void on_CenterPos_triggered(bool);
		void serverConnected();
		void serverDisconnected(QString reason);
		void pushLink(bool down);
	public:
		MainWindow(QWidget *parent);
};

#else
class MainWindow;
#endif
