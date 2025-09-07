#include "server/Channel.hpp"


Channel::Channel() : Password("") {}

Channel::Channel(const std::string &name, const std::string &p) : _name(name), Password(p), TopicText(""), InviteOnly(false), OnlyOpInvite(false), Topic(false), ThereIsLimit(false) {
    if (p.size() == 0)
        NoPassword = false;
    else
        NoPassword = true;
}

Channel::~Channel() {}


std::map<int, bool> &Channel::getMembers() {
    return (this->_members);
}

void Channel::addMember(int fd, bool permission) {
    _members[fd] = permission;
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
}

std::string Channel::getPassword(){ return Password; }
std::string Channel::getName(){ return _name; }
std::string Channel::getTopicText(){ return this->TopicText; }
bool Channel::getInviteOnly(){ return this->InviteOnly; }
bool Channel::getTopic(){ return this->Topic; }
bool Channel::getOnlyOpInvite(){ return this->OnlyOpInvite; }
bool Channel::getNoPassword(){ return this->NoPassword; }
bool Channel::getThereIsLimit(){ return this->ThereIsLimit; }

void Channel::setPassword(std::string &Password){ this->Password = Password; }
void Channel::setOnlyOpInvite(bool value){ this->OnlyOpInvite = value; }
void Channel::setInviteOnly(bool value){ this->InviteOnly = value; }
void Channel::setTopicText(std::string TopicText){ this->TopicText.clear(); this->TopicText = TopicText; }
void Channel::setNoPassword(bool v) { this->NoPassword = v; }
void Channel::setThereIsLimit(bool v) { this->ThereIsLimit = v; }

int Channel::OnList(int fd)
{
    std::vector<int>::iterator found_it = std::find(this->invited_members.begin(), this->invited_members.end(), fd);
    if (this->invited_members.size() > 0 && found_it != this->invited_members.end())
        return (0);
    return (1);
}

void Channel::AddToList(int fd)
{
    this->invited_members.push_back(fd);
}

int Channel::getLimit(){ return this->Limit; }
void Channel::setLimit(int num){ this->Limit = num; }
void Channel::setTopic(bool v){ this->Topic = v; }

