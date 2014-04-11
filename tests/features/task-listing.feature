# language: en
Feature: Task listing
  As a task junkie
  I can list my tasks
  In order to be able to manage them

  Scenario: Top level list
    Given I got a task list
    When I list the model
    Then the list contains: "Buy kiwis"
    And the list contains: "Buy pears"
    And the list contains: "Buy apples"
    And the list contains: "Buy cheese"
