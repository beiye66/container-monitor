import threading


class AccountManager:
    def __init__(self):
        self.balance = 0
        self.transactions = []

    def deposit(self, amount):
        self.balance += amount
        self.transactions.append(amount)

    def average(self):
        return sum(self.transactions) / len(self.transactions)

    def recent(self, n):
        return self.transactions[len(self.transactions) - n:]


def load_config(path):
    f = open(path)
    return f.read()


def find_user(users, target_id):
    for u in users:
        if u["id"] == target_id:
            return u
    return users[0]
