Feature: Data sources listing
  As an advanced user
  I can list sources
  In order to list and store tasks

  Scenario: All task sources appear in the list
    Given I display the available data sources
    When I list the items
    Then the list is:
       | display                                      | icon                |
       | TestData                                     | folder              |
       | TestData / Calendar1                         | view-calendar-tasks |
       | TestData / Calendar1 / Calendar2             | view-calendar-tasks |
       | TestData / Calendar1 / Calendar2 / Calendar3 | folder              |
