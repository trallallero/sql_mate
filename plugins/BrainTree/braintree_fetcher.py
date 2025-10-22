import braintree
import json
import sys
import getopt
from datetime import datetime


class BraintreeFetcher:
    def __init__(self, environment, merchant_id, pubkey, prikey):
        self.gateway = braintree.BraintreeGateway(
            braintree.Configuration(
                environment=environment,
                merchant_id=merchant_id,
                public_key=pubkey,
                private_key=prikey
            )
        )

    def get_transactions(self, sub_code):
        result = []

        try:
            subscription = self.gateway.subscription.find(sub_code)
        except Exception as e:
            print(f'Exception occurred: <{str(e)}>', file=sys.stderr)
            return result

        transactions = subscription.transactions

        for transaction in transactions:
            result.append({
                'status'                  : transaction.status,
                'transaction_id'          : transaction.id,
                'payment_instrument_type' : transaction.payment_instrument_type,
                'amount'                  : str(float(transaction.amount)),
                'processor_response_text' : transaction.processor_response_text,
                'type'                    : transaction.type,
                'created_at'              : datetime.strftime(transaction.created_at, '%Y-%m-%d'),
                'recurring'               : str(transaction.recurring),
                'subscription_id'         : transaction.subscription_id
            })
        return result


if __name__ == "__main__":
    try:
        if len(sys.argv) != 11:
            print("""USAGE: python braintree_fetcher.py
                -e environment
                -m merchant id
                -p public key
                -k private key
                -s subscription code""", file=sys.stderr)
            exit(1)
        opts, args = getopt.getopt(sys.argv[1:], "e:m:p:k:s:")
        for o, v in opts:
            if o == '-e':
                environment = v
            elif o == '-m':
                merchant_id = v
            elif o == '-p':
                pubkey = v
            elif o == '-k':
                prikey = v
            elif o == '-s':
                subcode = v
            else:
                assert False, "unhandled option"

        c = BraintreeFetcher(
            environment=environment,
            merchant_id=merchant_id,
            pubkey=pubkey,
            prikey=prikey)
        result = c.get_transactions(sub_code=subcode)
        json_object = json.dumps(result, indent=4)
        print(json_object)
        exit(0)
    except Exception as e:
        print(str(e), file=sys.stderr)
        exit(1)
