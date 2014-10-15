Feature: Data sources listing
  As an advanced user
  I can list sources
  In order to list and store artifacts

  Scenario: Flat task data sources
    Given I display the available task data sources
    When I list the items
    Then the list is:
       | display                                | icon                |
       | TestData/Calendar1                     | view-calendar-tasks |
       | TestData/Calendar1/Calendar2           | view-calendar-tasks |
       | TestData/Calendar1/Calendar2/Calendar3 | folder              |

  Scenario: Flat note data sources
    Given I display the available note data sources
    When I list the items
    Then the list is:
       | display                                | icon                |
       | TestData/Emails/Notes                  | folder              |
       | TestData/Private Notes                 | folder              |

  Scenario: All task and note sources appear in the list
    Given I display the available data sources
    When I list the items
    Then the list is:
       | display                                      | icon                |
       | TestData                                     | folder              |
       | TestData / Calendar1                         | view-calendar-tasks |
       | TestData / Calendar1 / Calendar2             | view-calendar-tasks |
       | TestData / Calendar1 / Calendar2 / Calendar3 | folder              |
       | TestData / Emails                            | folder              |
       | TestData / Emails / Notes                    | folder              |
       | TestData / Private Notes                     | folder              |
