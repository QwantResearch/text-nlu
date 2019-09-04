# coding: utf-8

import sys
sys.path.insert(1, '../compiled')

import grpc

import grpc_nlu_pb2
import grpc_nlu_pb2_grpc

channel = grpc.insecure_channel('localhost:9010') 
stub = grpc_nlu_pb2_grpc.RouteNLUStub(channel)

domains = stub.GetDomains(grpc_nlu_pb2.Empty())
print(domains)

text_to_nlu = grpc_nlu_pb2.TextToParse(
    text="pizza à Rennes",
    count=2,
    lang="fr",
    domain="POI")

def generate_to_nlu(): 
    to_nlu = [text_to_nlu] * 2
    to_nlu.append(grpc_nlu_pb2.TextToParse(
        text="le café de la gare en bretagne",
        count=2,
        lang="fr",
        domain="POI"))
        
    for elem in to_nlu: 
        yield elem    

parsed = stub.GetNLU(text_to_nlu)
print(parsed)

responses = stub.StreamNLU(generate_to_nlu())
for response in responses:
    print(response)
