from gi.repository import GHbci

import test_account

context = GHbci.Context.new()

def log(self, msg, level, data):
    print "log: ", msg

def answer(self, reason, msg, optional_data, data):
    print "event: ", reason, msg
    if reason == GHbci.Reason.NEED_COUNTRY:
        return "DE"
    if reason == GHbci.Reason.NEED_BLZ:
        return test_account.BLZ
    if reason == GHbci.Reason.NEED_USERID:
        return test_account.USERID
    if reason == GHbci.Reason.NEED_CUSTOMERID:
        return test_account.CUSTOMERID
    if reason == GHbci.Reason.NEED_HOST:
        return test_account.HOST
    if reason == GHbci.Reason.NEED_PORT:
        return test_account.PORT
    if reason == GHbci.Reason.NEED_FILTER:
        return "Base64"
    if reason == GHbci.Reason.NEED_PASSPHRASE_LOAD:
        return "42"
    if reason == GHbci.Reason.NEED_PASSPHRASE_SAVE:
        return "42"
    if reason == GHbci.Reason.NEED_PT_PIN:
        return test_account.PIN
    if reason == GHbci.Reason.NEED_PT_SECMECH:
        print optional_data
        return test_account.SECMECH
    return ""

context.connect("callback", answer, None)
context.connect("log", log, None)

print context.add_passport(test_account.BLZ, test_account.USERID)

accounts = context.get_accounts(test_account.BLZ, test_account.USERID)
print accounts
for account in accounts:
    print account.get_property('number')

print '"'+context.get_balances(test_account.BLZ, test_account.USERID, test_account.ACCOUNT_NUMBER)+'"'

l = context.get_statements(test_account.BLZ, test_account.USERID, test_account.ACCOUNT_NUMBER)
for s in l:
    valuta = s.get_property('valuta')
    print "{}. {}. {}".format(valuta.get_day(), valuta.get_month(), valuta.get_year())
    bdate = s.get_property('booking-date')
    print "{}. {}. {}".format(bdate.get_day(), bdate.get_month(), bdate.get_year())
    print s.get_property('saldo')
    print s.get_property('value')
    print s.get_property('reference')
    print s.get_property('gv-code')
    print s.get_property('other-name')
    print s.get_property('other-bic')
    print s.get_property('other_iban')

#bank = context.get_name_for_blz("50010517")
#print bank

#def callback(blz, user_data):
#    print blz, context.get_name_for_blz(blz)

#context.blz_foreach(callback, "test")
