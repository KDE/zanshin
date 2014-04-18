# language: en
Feature: Task listing
  As a task junkie
  I can list my tasks
  In order to be able to manage them

  Scenario: Top level list
    Given I got a task list
    When I list the model
    Then the list is:
       | display    |
       | Buy kiwis  |
       | Buy pears  |
       | Buy apples |
       | Buy cheese |
