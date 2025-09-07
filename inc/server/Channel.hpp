#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <map>
#include <vector>
#include <algorithm>

class Channel
{
	private:
		std::string			_name;
		std::map<int, bool> _members;
		std::vector<int>    invited_members;
		std::string         Password;
		std::string         TopicText;
		bool		        NoPassword;
		bool                InviteOnly;
		bool				OnlyOpInvite;
		bool                Topic;
		bool				ThereIsLimit;
		int					Limit;
	public:
		Channel();
		Channel(const std::string& name, const std::string& p);
		~Channel();

		void addMember(int fd, bool permission);
		void removeMember(int fd);
		std::map<int, bool> &getMembers();
		std::string getPassword();
		std::string getName();
		bool        getTopic();
		int        OnList(int fd);
		std::string getTopicText();
		bool        getInviteOnly();
		void        setInviteOnly(bool value);
		void        setOnlyOpInvite(bool value);
		bool		getOnlyOpInvite();
		void        setPassword(std::string &password);
		void        setTopicText(std::string text);
		void		AddToList(int fd);
		void		setNoPassword(bool value);
		bool		getNoPassword();
		void		setThereIsLimit(bool value);
		bool		getThereIsLimit();
		int			getLimit();
		void		setLimit(int num);
		void		setTopic(bool v);
};

#endif
