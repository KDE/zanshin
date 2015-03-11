Feature: Workday content
  As someone using tasks
  I can display the Workday list
  In order to know which tasks should be completed today (e.g. if start date or due date is today or in the past)

@wip
  Scenario: The tasks that need to be done today appear in the Workday list
    Given I display the "Workday" page
    And I look at the central list
    When I list the items
    Then the list is :
       | display           | start date    | due date     |
       | Buy milk          | 10 days ago   | today        |
       | Go to the library | today         | in 10 days   |
       | Learn Caml        | 4 years ago   | 3 years ago  |
       | Learn Perl        | today         | today        |
