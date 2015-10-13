#!/usr/bin/env python

class A(object):

  class_field = 42

  def __init__(self):
    print self.__class__.class_field


def main():
  a = A()

if __name__ == '__main__':
  main()
