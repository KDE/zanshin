Feature: Data sources listing
  As an advanced user
  I can list sources
  In order to list and store artifacts

  Scenario: Flat task data sources
    Given I got a task data source list model
    When I list the model
    Then the list is:
       | display                                | icon                |
       | TestData/Calendar1                     | view-calendar-tasks |
       | TestData/Calendar1/Calendar2           | view-calendar-tasks |
       | TestData/Calendar1/Calendar2/Calendar3 | folder              |

  Scenario: Flat note data sources
    Given I got a note data source list model
    When I list the model
    Then the list is:
       | display                                | icon                |
       | TestData/Emails/Notes                  | folder              |
       | TestData/Private Notes                 | folder              |
